#include "UploadQueue.h"

#include "IRendererBackend.h"
#include "IResourceManager.h"

#include "Fence.h"
#include "Barrier.h"
#include "Resource.h"
#include "CommandQueue.h"
#include "CommandBuffer.h"

namespace Cyclone::Render
{

CUploadQueue::CUploadQueue() = default;
CUploadQueue::~CUploadQueue()
{
    DeInit();
}

C_STATUS CUploadQueue::Init(const CUploadQueueDesc& Desc)
{
    // Here implemented buffer suballocation approach using ring-buffer with persistent mapping
    // #todo_upload implement heap suballocation approach

    C_STATUS Result = C_STATUS::C_STATUS_OK;

    m_Desc = Desc;

    IResourceManager* ResourceManager = m_Desc.Backend->GetResourceManager(m_Desc.DeviceHandle);
    CASSERT(ResourceManager);

    CResourceDesc ResourceDesc{};
    ResourceDesc.Backend = m_Desc.Backend;
    ResourceDesc.DeviceHandle = m_Desc.DeviceHandle;
    ResourceDesc.Flags = EResourceFlags::Buffer | EResourceFlags::Mappable | EResourceFlags::DenyShaderResource | EResourceFlags::HeapTypeUpload;
    ResourceDesc.Buffer.Usage = EBufferUsageType::TransferDst | EBufferUsageType::TransferSrc;
    ResourceDesc.Buffer.ByteSize = m_Desc.BufferSize;
    ResourceDesc.InitialLayout = EImageLayoutType::ReadOnly;
    
#if ENABLE_DEBUG_RENDER_BACKEND
    static uint32 counter = 0;
    ResourceDesc.Name = "UploadBuffer" + ToString(counter++);
#endif

    m_Buffer = ResourceManager->CreateResource(ResourceDesc);
    C_ASSERT_RETURN_VAL(m_Buffer.IsValid(), C_STATUS::C_STATUS_ERROR);

    CResource* BufferPtr = ResourceManager->GetResource(m_Buffer);
    C_ASSERT_RETURN_VAL(BufferPtr, C_STATUS::C_STATUS_ERROR);

    // Do persistent map
    m_BufferMapData = BufferPtr->Map();
    CASSERT(m_BufferMapData);
    if (!m_BufferMapData)
    {
        return C_STATUS::C_STATUS_ERROR;
    }

    uint8* Data = static_cast<uint8*>(m_BufferMapData.Memory);
    Result = m_RingBuffer.Init(Data, Data, Data, Data + m_Desc.BufferSize);
    if (C_SUCCEEDED(Result) == false)
    {
        BufferPtr->UnMap(m_BufferMapData);
        return Result;
    }

    {
        CFenceDesc FenceDesc{};
        FenceDesc.Backend = m_Desc.Backend;
        FenceDesc.DeviceHandle = m_Desc.DeviceHandle;
        FenceDesc.InitialValue = 0;

#if ENABLE_DEBUG_RENDER_BACKEND
        static uint32 counter = 0;
        FenceDesc.Name = "UploadBufferFence" + ToString(counter++);
#endif

        m_Fence = ResourceManager->CreateFence(FenceDesc);
        C_ASSERT_RETURN_VAL(m_Fence.IsValid(), C_STATUS::C_STATUS_ERROR);
    }

    return Result;
}

// #todo_upload return handle to track completion
C_STATUS CUploadQueue::QueueUploadData(const void* Data, uint64 NumBytes, CHandle<CResource> DstResource, 
    uint64 DstResourceByteOffset, EImageLayoutType DstResourceDesiredLayoutAfterTransfer,
    EImageUsageType DstResourceDesiredUsageAfterTransfer,
    uint32 Alignment, uint16 FirstSubresource, int16 SubresourcesCount)
{
    C_ASSERT_RETURN_VAL(NumBytes <= m_Desc.BufferSize, C_STATUS::C_STATUS_OUT_OF_MEM);
    
    IResourceManager* ResourceManager = m_Desc.Backend->GetResourceManager(m_Desc.DeviceHandle);
    CASSERT(ResourceManager);
    CASSERT(m_Fence.IsValid());
    CFence* FencePtr = ResourceManager->GetFence(m_Fence);
    CASSERT(FencePtr);

    uint8* DstDataPtr = nullptr;

    if (!C_SUCCEEDED(m_RingBuffer.Allocate(NumBytes, Alignment, &DstDataPtr)))
    {
        // Try to find free memory and wait for queue completion if needed
        uint64 CompletedValue = FencePtr->GetCompletedValue();
        ReleaseMemoryUntilFence(CompletedValue);

        const uint32 MaxTriesCount = 10;
        uint32 TriesCount = 0;

        while (!C_SUCCEEDED(m_RingBuffer.Allocate(NumBytes, Alignment, &DstDataPtr)) && !m_FrameOffsetQueue.empty())
        {
            // Enqueue pending uploads
            CCommandBuffer* CommandBuffer = m_Desc.CommandQueue->AllocateCommandBuffer();
            C_ASSERT_RETURN_VAL(CommandBuffer, C_STATUS::C_STATUS_INVALID_ARG);

            C_STATUS Result = CommandBuffer->Begin();
            C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

            Result = EnqueueGPUWork(CommandBuffer);
            C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

            // Flush GPU work
            CommandBuffer->End();

            m_Desc.CommandQueue->Submit(&CommandBuffer, 1, true);
            C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

            // #todo_vk potential deadlock from Presentation logic
            FencePtr->IncrementAndSignalFromDevice(m_Desc.CommandQueue);
            FencePtr->WaitFromHost(FencePtr->GetValue());

            ReleaseMemoryUntilFence(FencePtr->GetCompletedValue());

            ++TriesCount;

            if (TriesCount >= 10)
            {
                CASSERT(false && "Can't find free space in UploadBuffer");
                return C_STATUS::C_STATUS_OUT_OF_MEM;
            }
        }
    }

    if (DstDataPtr == nullptr)
        return C_STATUS::C_STATUS_OUT_OF_MEM;

    // Copy data to upload buffer
    {
#if 0
        CD3DX12_RANGE readRange(0, 0);
        uint8* dstPtr = nullptr;
        C_STATUS res = m_resource->GetResource()->Map(0, &readRange, reinterpret_cast<void**>(&dstPtr));
        memcpy(dstPtr + (m_pCurr - m_pBegin), pData, NumBytes);
        m_resource->GetResource()->Unmap(0, &readRange);
#else
        memcpy(DstDataPtr, Data, NumBytes);
#endif
    }

    // Add allocation track
    uint64 FenceValueForTracking = FencePtr->GetValue() + 1;
    if (m_FrameOffsetQueue.empty() || m_FrameOffsetQueue.back().FenceValue < FenceValueForTracking)
    {
        m_FrameOffsetQueue.push({ DstDataPtr, FenceValueForTracking });
    }

    // Add task to queue
    uint64 BufferOffset = static_cast<uint64>(DstDataPtr - m_RingBuffer.GetBegin());
    EImageLayoutType LayoutAfter = DstResourceDesiredLayoutAfterTransfer;
    EImageUsageType UsageAfter = DstResourceDesiredUsageAfterTransfer;
    if (LayoutAfter == EImageLayoutType::Count || UsageAfter == EImageUsageType::Count)
    {
        CResource* DstResourcePtr = m_Desc.Backend->GetResourceManager(m_Desc.DeviceHandle)->GetResource(DstResource);
        CASSERT(DstResourcePtr);

        if (LayoutAfter == EImageLayoutType::Count)
            LayoutAfter = DstResourcePtr->GetTraceableLayout();
        if (UsageAfter == EImageUsageType::Count)
            UsageAfter = DstResourcePtr->GetTraceableUsageType();
    }

    m_UploadQueue.emplace_back(CCopyEntry{ BufferOffset, DstResourceByteOffset, NumBytes, LayoutAfter, UsageAfter, DstResource, FirstSubresource, SubresourcesCount });

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CUploadQueue::EnqueueGPUWork(CCommandBuffer* CommandBuffer)
{
    if (m_UploadQueue.empty() || m_ReentranceFlag)
        return C_STATUS::C_STATUS_OK;
    
    m_ReentranceFlag = true;

    IResourceManager* ResourceManager = m_Desc.Backend->GetResourceManager(m_Desc.DeviceHandle);
    CASSERT(ResourceManager);

    auto InsertBarrier = [CommandBuffer, ResourceManager, this](const CCopyEntry& Entry, EImageLayoutType PrevState, EImageLayoutType NewState, EImageUsageType NewUsage)
    {
        CPipelineBarrier Barrier{};
        // #todo_upload #todo_vk Host_STAGE_BIT as execution dependency
        
        CResource* ResourcePtr = ResourceManager->GetResource(Entry.DstResource);
        CASSERT(ResourcePtr);
        if (ResourcePtr->GetDesc().Flags & EResourceFlags::Texture)
        {
            // #todo_vk_refactor optimize barriers
            Barrier = CPipelineBarrier::FromTexture(Entry.DstResource, PrevState, NewState, {},
                EExecutionStageMask::AllCommands, EMemoryAccessMask::ShaderWrite | EMemoryAccessMask::ColorAttachmentWrite | EMemoryAccessMask::TransferWrite,
                EExecutionStageMask::AllCommands, EMemoryAccessMask::ColorAttachmentRead | EMemoryAccessMask::ShaderRead | EMemoryAccessMask::TransferRead | EMemoryAccessMask::ShaderWrite | EMemoryAccessMask::ColorAttachmentWrite | EMemoryAccessMask::TransferWrite);
            Barrier.NewUsage = NewUsage;
        }
        else if (ResourcePtr->GetDesc().Flags & EResourceFlags::Buffer)
        {
            Barrier.Action = EPipelineBarrierAction::BufferMemory;
            Barrier.Resource = Entry.DstResource;
            Barrier.Offset = Entry.DstOffset;
            Barrier.Size = Entry.NumBytes;
            Barrier.SrcPipelineStageMask = EExecutionStageMask::AllCommands;
            Barrier.SrcMemoryAccessMask = EMemoryAccessMask::ShaderWrite | EMemoryAccessMask::ColorAttachmentWrite | EMemoryAccessMask::TransferWrite;
            Barrier.DstPipelineStageMask = EExecutionStageMask::AllCommands;
            Barrier.DstMemoryAccessMask = EMemoryAccessMask::ShaderRead | EMemoryAccessMask::ColorAttachmentRead | EMemoryAccessMask::TransferRead | EMemoryAccessMask::ShaderWrite | EMemoryAccessMask::ColorAttachmentWrite | EMemoryAccessMask::TransferWrite;
            Barrier.NewUsage = NewUsage;
        }
        else
        {
            CASSERT(false && "Unsupported resource type for barrier");
        }

        CommandBuffer->InsertPipelineBarrier(MoveTemp(Barrier));
    };

    // Insert barriers for transition to copy dest state
    for (auto& Entry : m_UploadQueue)
    {
        InsertBarrier(Entry, EImageLayoutType::Undefined, EImageLayoutType::TransferDst, EImageUsageType::TransferDst);
    }

    // Barriers should be submitted automatically
    //CommandBuffer->FLushPendingPipelineBarriers();

    // read all upload entries and enqueue them in command list
    // each resource also transited with barrier to COPY_DESC and back
    for (auto& Entry : m_UploadQueue)
    {
        CASSERT(Entry.DstResource.IsValid());
        IResourceManager* ResourceManager = m_Desc.Backend->GetResourceManager(CDeviceHandle::From(Entry.DstResource));

        CResource* DstResource = ResourceManager->GetResource(Entry.DstResource);
        CASSERT(DstResource);

        if (DstResource->GetDesc().Flags & EResourceFlags::Buffer)
        {
            CCopyBufferDesc CopyDesc{};
            CopyDesc.SrcBuffer = m_Buffer;
            CopyDesc.DstBuffer = Entry.DstResource;
            CopyDesc.SrcOffset = Entry.SrcOffset;
            CopyDesc.DstOffset = Entry.DstOffset;
            CopyDesc.NumBytes = Entry.NumBytes;
            CopyDesc.FirstSubresource = Entry.FirstSubresource;
            CopyDesc.SubresourcesCount = Entry.SubresourcesCount;
            CommandBuffer->CopyBuffer(CopyDesc);
        }
        else if (DstResource->GetDesc().Flags & EResourceFlags::Texture)
        {
            CCopyBufferToTextureDesc CopyDesc{};
            CopyDesc.SrcBuffer = m_Buffer;
            CopyDesc.DstTexture = Entry.DstResource;
            CopyDesc.SrcBaseOffset = Entry.SrcOffset;
            CopyDesc.DstBaseOffset = Entry.DstOffset;
            CopyDesc.NumBytes = Entry.NumBytes;
            CopyDesc.FirstSubresource = Entry.FirstSubresource;
            CopyDesc.SubresourcesCount = Entry.SubresourcesCount;

            // #todo_vk #todo_upload array of subresources + refactor
            CommandBuffer->CopyBufferToTexture(CopyDesc);
        }
        else
        {
            CASSERT(false);
        }
    }

    // Insert barriers to switch resources to its previous state
    for (auto& Entry : m_UploadQueue)
    {
        InsertBarrier(Entry, EImageLayoutType::TransferDst, Entry.DstResourceStateAfter, Entry.DstResourceUsageAfter);
    }

    // Barriers should be submitted automatically
    //CommandBuffer->FLushPendingPipelineBarriers();

    m_UploadQueue.clear();

    m_ReentranceFlag = false;

    return C_STATUS::C_STATUS_OK;
}

void CUploadQueue::ReleaseMemoryUntilFence(uint64 LastCompletedFence)
{
    while (!m_FrameOffsetQueue.empty() && m_FrameOffsetQueue.front().FenceValue <= LastCompletedFence)
    {
        uint8* CurrOccupied = m_FrameOffsetQueue.front().Data;
        m_RingBuffer.SetCurrOccupied(CurrOccupied);
        m_FrameOffsetQueue.pop();
    }

    if (m_FrameOffsetQueue.empty())
    {
        m_RingBuffer.SetCurrOccupied(m_RingBuffer.GetCurrFree());
    }
}

void CUploadQueue::DeInit() noexcept
{
    CASSERT(m_UploadQueue.empty());

    if (m_Buffer.IsValid())
    {
        CResource* BufferPtr = m_Desc.Backend->GetResourceManager(m_Desc.DeviceHandle)->GetResource(m_Buffer);
        CASSERT(BufferPtr);

        if (BufferPtr)
        {
            BufferPtr->UnMap(m_BufferMapData);
        }

        m_Desc.Backend->GetResourceManager(m_Desc.DeviceHandle)->DestroyResource(m_Buffer);
        m_Buffer = {};
    }

    m_RingBuffer.Init(nullptr, nullptr, nullptr, nullptr);
}

} // namespace Cyclone::Render
