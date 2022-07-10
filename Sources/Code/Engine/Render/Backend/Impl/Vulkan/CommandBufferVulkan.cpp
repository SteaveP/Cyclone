#include "CommandBufferVulkan.h"

#include "Engine/Render/Backend/UploadQueue.h"
#include "Engine/Render/Backend/Barrier.h"

#include "CommandQueueVulkan.h"
#include "RenderBackendVulkan.h"
#include "ResourceVulkan.h"
#include "ResourceViewVulkan.h"

#include "Internal/PipelineStateVulkan.h"

#include <type_traits>

namespace Cyclone::Render
{

CCommandBufferVulkan::CCommandBufferVulkan() = default;
CCommandContextVulkan::CCommandContextVulkan() = default;

CCommandBufferVulkan::CCommandBufferVulkan(CCommandBufferVulkan&& Other) noexcept : CCommandBuffer(MoveTemp(Other))
{
    std::swap(m_CommandQueueVk, Other.m_CommandQueueVk);
    std::swap(m_BackendVk, Other.m_BackendVk);
    std::swap(m_ContextVk, Other.m_ContextVk);
    std::swap(m_CommandPool, Other.m_CommandPool);
    std::swap(m_CommandBuffer, Other.m_CommandBuffer);
    std::swap(m_CompleteSemaphore, Other.m_CompleteSemaphore);
}
CCommandBufferVulkan& CCommandBufferVulkan::operator=(CCommandBufferVulkan&& Other) noexcept
{
    if (this != &Other)
    {
        CCommandBuffer::operator=(MoveTemp(Other));
        std::swap(m_CommandQueueVk, Other.m_CommandQueueVk);
        std::swap(m_BackendVk, Other.m_BackendVk);
        std::swap(m_ContextVk, Other.m_ContextVk);
        std::swap(m_CommandPool, Other.m_CommandPool);
        std::swap(m_CommandBuffer, Other.m_CommandBuffer);
        std::swap(m_CompleteSemaphore, Other.m_CompleteSemaphore);
    }
    return *this;
}

CCommandBufferVulkan::~CCommandBufferVulkan()
{
    DeInitImpl();
}

CCommandContextVulkan::~CCommandContextVulkan()
{
    DeInitImpl();
}

C_STATUS CCommandBufferVulkan::Init(CCommandQueueVulkan* CommandQueue)
{
    m_CommandQueue = CommandQueue;
    m_CommandQueueVk = CommandQueue;
    m_Backend = m_CommandQueueVk->GetBackendVk();
    m_BackendVk = m_CommandQueueVk->GetBackendVk();
    m_DeviceHandle = m_CommandQueueVk->GetDeviceHandle();
    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(GetDeviceHandle());

    // Create semaphore
    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkResult ResultVk = VK_CALL(Device, vkCreateSemaphore(Device.DeviceVk, &SemaphoreInfo, nullptr, &m_CompleteSemaphore));
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);

    // Create command pool
    VkCommandPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.queueFamilyIndex = m_CommandQueueVk->GetQueueFamilyIndex();
    PoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // Optional

    ResultVk = VK_CALL(Device, vkCreateCommandPool(Device.DeviceVk, &PoolInfo, nullptr, &m_CommandPool));
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);

    // Create command buffer
    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = m_CommandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = 1;

    ResultVk = VK_CALL(Device, vkAllocateCommandBuffers(Device.DeviceVk, &AllocInfo, &m_CommandBuffer));
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);

    // Command Context
    auto CommandContextVk = MakeUnique<CCommandContextVulkan>();
    C_STATUS Result = CommandContextVk->Init(GetBackendVk(), this);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    m_ContextVk = CommandContextVk.get();
    m_Context = MoveTemp(CommandContextVk);

    return C_STATUS::C_STATUS_OK;
}

void CCommandBufferVulkan::DeInit()
{
    DeInitImpl();
    CCommandBuffer::DeInit();
}

void CCommandBufferVulkan::DeInitImpl()
{
    if (GetBackendVk() == nullptr)
        return;

    GetBackendVk()->GetDisposalManagerVk(GetDeviceHandle())->AddDisposable([Backend = GetBackendVk(), DeviceHandle = GetDeviceHandle(),
        Sem = m_CompleteSemaphore, Buf = m_CommandBuffer, Pool = m_CommandPool]()
    {
        const auto& Device = Backend->GetDeviceManager().GetDevice(DeviceHandle);

        if (Sem)
            VK_CALL(Device, vkDestroySemaphore(Device.DeviceVk, Sem, nullptr));

        if (Pool && Buf)
            VK_CALL(Device, vkFreeCommandBuffers(Device.DeviceVk, Pool, 1, &Buf));

        if (Pool)
            VK_CALL(Device, vkDestroyCommandPool(Device.DeviceVk, Pool, nullptr));
    });
        
    m_CompleteSemaphore = VK_NULL_HANDLE;
    m_CommandBuffer = VK_NULL_HANDLE;
    m_CommandPool = VK_NULL_HANDLE;
    m_BackendVk = nullptr;
}

C_STATUS CCommandBufferVulkan::Begin()
{
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    auto& Device = GetBackendVk()->GetDeviceManager().GetDevice(GetDeviceHandle());

    VkResult ResultVk = VK_CALL(Device, vkBeginCommandBuffer(m_CommandBuffer, &BeginInfo));
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);

    C_STATUS Result = CCommandBuffer::Begin();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

void CCommandBufferVulkan::End()
{
    CCommandBuffer::End();

    auto& Device = GetBackendVk()->GetDeviceManager().GetDevice(GetDeviceHandle());

    VkResult Result = VK_CALL(Device, vkEndCommandBuffer(m_CommandBuffer));
    C_ASSERT_VK_SUCCEEDED(Result);
}

void CCommandBufferVulkan::Reset()
{
    auto& Device = GetBackendVk()->GetDeviceManager().GetDevice(GetDeviceHandle());

    VkResult Result = VK_CALL(Device, vkResetCommandPool(Device.DeviceVk, m_CommandPool, 0));
    C_ASSERT_VK_SUCCEEDED(Result);
}

void CCommandBufferVulkan::Draw(uint32 IndexCount, uint32 InstanceCount, uint32 FirstIndex, int32 VertexOffset, uint32 FirstInstance)
{
    CCommandBuffer::Draw(IndexCount, InstanceCount, FirstIndex, VertexOffset, FirstInstance);
    
    auto& Device = GetBackendVk()->GetDeviceManager().GetDevice(GetDeviceHandle());

    m_Context->FLushPendingPipelineBarriers(this);

    VK_CALL(Device, vkCmdDrawIndexed(m_CommandBuffer, IndexCount, InstanceCount, FirstIndex, VertexOffset, FirstInstance));
}

void CCommandBufferVulkan::CopyBuffer(const CCopyBufferDesc& Desc)
{
    CCommandBuffer::CopyBuffer(Desc);

    CResourceManagerVulkan* ResourceManagerVk = GetBackendVk()->GetResourceManagerVk(CDeviceHandle::From(Desc.SrcBuffer));
    CASSERT(ResourceManagerVk);

    CResourceVulkan* SrcBufPtr = BACKEND_DOWNCAST(ResourceManagerVk->GetResource(Desc.SrcBuffer), CResourceVulkan);
    CResourceVulkan* DstBufPtr = BACKEND_DOWNCAST(ResourceManagerVk->GetResource(Desc.DstBuffer), CResourceVulkan);
    CASSERT(SrcBufPtr && DstBufPtr);

    CASSERT(SrcBufPtr->GetDesc().Flags & EResourceFlags::Buffer);
    CASSERT(DstBufPtr->GetDesc().Flags & EResourceFlags::Buffer);

    VkCopyBufferInfo2 Info{ .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2 };
    Info.srcBuffer = SrcBufPtr->GetBuffer();
    Info.dstBuffer = DstBufPtr->GetBuffer();

    VkBufferCopy2 CopyInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
        .srcOffset = Desc.SrcOffset,
        .dstOffset = Desc.DstOffset,
        .size = Desc.NumBytes
    };

    Info.regionCount = 1;
    Info.pRegions = &CopyInfo;

    auto& Device = GetBackendVk()->GetDeviceManager().GetDevice(GetDeviceHandle());
    VK_CALL(Device, vkCmdCopyBuffer2(Get(), &Info));
}

// note that this function can't be called inside RenderPass
void CCommandBufferVulkan::CopyBufferToTexture(const CCopyBufferToTextureDesc& Desc)
{
    CCommandBuffer::CopyBufferToTexture(Desc);

    auto& Device = GetBackendVk()->GetDeviceManager().GetDevice(GetDeviceHandle());
    CResourceManagerVulkan* ResourceManagerVk = Device.ResourceManager.get();
    CASSERT(ResourceManagerVk);

    CResourceVulkan* SrcBufPtr = BACKEND_DOWNCAST(ResourceManagerVk->GetResource(Desc.SrcBuffer), CResourceVulkan);
    CResourceVulkan* DstTexPtr = BACKEND_DOWNCAST(ResourceManagerVk->GetResource(Desc.DstTexture), CResourceVulkan);
    CASSERT(SrcBufPtr && DstTexPtr);

    CASSERT(SrcBufPtr->GetDesc().Flags & EResourceFlags::Buffer);
    CASSERT(DstTexPtr->GetDesc().Flags & EResourceFlags::Texture);

    VkCopyBufferToImageInfo2 Info{ .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2 };
    Info.srcBuffer = SrcBufPtr->GetBuffer();
    Info.dstImage = DstTexPtr->GetImage();
    Info.dstImageLayout = ConvertLayoutType(DstTexPtr->GetTraceableLayout());

    constexpr uint32 MaxSubresources = 20;

    uint32 FirstSubresource = Desc.FirstSubresource;
    uint32 SubresourcesCount = Desc.SubresourcesCount;
    if (SubresourcesCount < 0)
        SubresourcesCount = DstTexPtr->GetDesc().Texture.MipLevels - FirstSubresource;
    CASSERT(FirstSubresource + SubresourcesCount <= MaxSubresources);

    VkBufferImageCopy2 CopyInfo[MaxSubresources]{};

    uint64 BufferOffset = Desc.SrcBaseOffset;
    for (uint32 i = 0; i < SubresourcesCount; ++i)
    {
        // #todo_vk #todo_upload
        CASSERT(DstTexPtr->GetDesc().Format == EFormatType::RGBA8_SRGB);

        CopyInfo[i].sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
        CopyInfo[i].bufferOffset = BufferOffset;

        CopyInfo[i].bufferRowLength = 0;
        CopyInfo[i].bufferImageHeight = 0;
        CopyInfo[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        CopyInfo[i].imageSubresource.mipLevel = i;
        CopyInfo[i].imageSubresource.baseArrayLayer = 0; // #todo_vk_cubemap 
        CopyInfo[i].imageSubresource.layerCount = 1;
        CopyInfo[i].imageOffset = VkOffset3D{};
        CopyInfo[i].imageExtent = VkExtent3D{
            .width  = DstTexPtr->GetDesc().Texture.Width >> i,
            .height = DstTexPtr->GetDesc().Texture.Height >> i, 
            .depth  = 1
        };

        BufferOffset += CopyInfo[i].imageExtent.width * CopyInfo[i].imageExtent.height * sizeof(uint32);
    }
    Info.regionCount = SubresourcesCount;
    Info.pRegions = CopyInfo;

    VK_CALL(Device, vkCmdCopyBufferToImage2(Get(), &Info));
}

void CCommandContextVulkan::BeginRenderPass(CCommandBuffer* CommandBuffer, const CRenderPass& RenderPass)
{
    CCommandContext::BeginRenderPass(CommandBuffer, RenderPass);

    m_ActivePipelineState = {};

    CCommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CCommandBufferVulkan);

    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(CommandBufferVk->GetDeviceHandle());

    VkRenderingInfo Info{ .sType = VK_STRUCTURE_TYPE_RENDERING_INFO };
    VkRenderingAttachmentInfo AttachmentInfos[10]{};
    VkRenderingAttachmentInfo DepthAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    {
        for (uint32 i = 0; i < RenderPass.RenderTargetSet.RenderTargetsCount; ++i)
        {
            auto& Att = AttachmentInfos[i];
            const auto& Src = RenderPass.RenderTargetSet.RenderTargets[i];
            Att.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            Att.loadOp = ConvertLoadOpType(Src.LoadOp);
            Att.storeOp = ConvertStoreOpType(Src.StoreOp);

            CResourceViewVulkan* ResourceViewPtr = BACKEND_DOWNCAST(Device.ResourceManager->GetResourceView(Src.RenderTarget.RenderTargetView), CResourceViewVulkan);
            CASSERT(ResourceViewPtr);

            Att.imageView = ResourceViewPtr->GetTextureView();

            CResource* ResourcePtr = Device.ResourceManager->GetResource(ResourceViewPtr->GetResource());
            CASSERT(ResourcePtr);

            Att.imageLayout = ConvertLayoutType(ResourcePtr->GetTraceableLayout());

            const auto& ClearValue = Src.ClearValue;
            Att.clearValue.color = VkClearColorValue{
                ClearValue.Color.X, ClearValue.Color.Y, ClearValue.Color.Z, ClearValue.Color.W };
        }

        Info.colorAttachmentCount = RenderPass.RenderTargetSet.RenderTargetsCount;
        Info.pColorAttachments = AttachmentInfos;
        Info.layerCount = 1;
        if (RenderPass.RenderTargetSet.DepthScentil.IsValid())
        {
            auto& Att = DepthAttachment;
            const auto& Src = RenderPass.RenderTargetSet.DepthScentil;

            Att.loadOp = ConvertLoadOpType(Src.LoadOp);
            Att.storeOp = ConvertStoreOpType(Src.StoreOp);

            CResourceViewVulkan* ResourceViewPtr = BACKEND_DOWNCAST(Device.ResourceManager->GetResourceView(Src.RenderTarget.DepthStencilView), CResourceViewVulkan);
            CASSERT(ResourceViewPtr);

            Att.imageView = ResourceViewPtr->GetTextureView();

            CResource* ResourcePtr = Device.ResourceManager->GetResource(ResourceViewPtr->GetResource());
            CASSERT(ResourcePtr);

            Att.imageLayout = ConvertLayoutType(ResourcePtr->GetTraceableLayout());

            const auto& ClearValue = Src.ClearValue;
            Att.clearValue.depthStencil = VkClearDepthStencilValue{ ClearValue.Color.X, (uint32)ClearValue.Color.Y};
            Info.pDepthAttachment = &Att;

            Info.pStencilAttachment = nullptr; // #todo_vk_stencil
        }
        Info.renderArea.offset = { (int32)RenderPass.ViewportExtent.X,  (int32)RenderPass.ViewportExtent.Y };
        Info.renderArea.extent = { (uint32)(RenderPass.ViewportExtent.Z - RenderPass.ViewportExtent.X), (uint32)(RenderPass.ViewportExtent.W - RenderPass.ViewportExtent.Y) };

        VK_CALL(Device, vkCmdBeginRendering(CommandBufferVk->Get(), &Info));
    }

#if 1
// #todo_vk This should be commands to CommandBuffer
    VkViewport viewport{};
    viewport.x = (float)Info.renderArea.offset.x;
    viewport.y = (float)Info.renderArea.offset.y;
    viewport.width = (float)Info.renderArea.extent.width;
    viewport.height = (float)Info.renderArea.extent.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    VK_CALL(Device, vkCmdSetViewport(CommandBufferVk->Get(), 0, 1, &viewport));

    VkRect2D scissor = Info.renderArea;
    VK_CALL(Device, vkCmdSetScissor(CommandBufferVk->Get(), 0, 1, &scissor));
#endif
}

void CCommandContextVulkan::EndRenderPass(CCommandBuffer* CommandBuffer)
{
    CASSERT(m_PendingBarriers.empty());

    CCommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CCommandBufferVulkan);
    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(CommandBuffer->GetDeviceHandle());

    VK_CALL(Device, vkCmdEndRendering(CommandBufferVk->Get()));

    m_ActivePipelineState = {};

    CCommandContext::EndRenderPass(CommandBuffer);
}

void CCommandContextVulkan::SetIndexBuffer(CCommandBuffer* CommandBuffer, CHandle<CResource> IndexBuffer, uint64 Offset, EFormatType Format)
{
    CCommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CCommandBufferVulkan);

    CResourceManagerVulkan* ResourceManagerVk = m_BackendVk->GetResourceManagerVk(CDeviceHandle::From(IndexBuffer));
    CASSERT(ResourceManagerVk);

    CResourceVulkan* IndexBufferPtr = BACKEND_DOWNCAST(ResourceManagerVk->GetResource(IndexBuffer), CResourceVulkan);
    CASSERT(IndexBufferPtr);

    CASSERT(IndexBufferPtr->GetDesc().Flags & EResourceFlags::Buffer);
    CASSERT(IndexBufferPtr->GetDesc().Buffer.Usage & EBufferUsageType::Index);
    CASSERT(Format == EFormatType::R16_UINT || Format == EFormatType::R32_UINT);
    
    VkIndexType IndexType = Format == EFormatType::R16_UINT ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(CommandBufferVk->GetDeviceHandle());
    VK_CALL(Device, vkCmdBindIndexBuffer(CommandBufferVk->Get(), IndexBufferPtr->GetBuffer(), Offset, IndexType));
}

void CCommandContextVulkan::SetVertexBuffer(CCommandBuffer* CommandBuffer, CHandle<CResource> VertexBuffer, uint32 Slot, uint64 Offset)
{
    CCommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CCommandBufferVulkan);
    
    CResourceManagerVulkan* ResourceManagerVk = m_BackendVk->GetResourceManagerVk(CDeviceHandle::From(VertexBuffer));
    CASSERT(ResourceManagerVk);

    CResourceVulkan* VertexBufferPtr = BACKEND_DOWNCAST(ResourceManagerVk->GetResource(VertexBuffer), CResourceVulkan);
    CASSERT(VertexBufferPtr);

    CASSERT(VertexBufferPtr->GetDesc().Flags & EResourceFlags::Buffer);
    CASSERT(VertexBufferPtr->GetDesc().Buffer.Usage & EBufferUsageType::Vertex);

    VkBuffer VertexBuffers[] = { VertexBufferPtr->GetBuffer() };
    VkDeviceSize Offests[] = { Offset };
    
    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(CommandBufferVk->GetDeviceHandle());
    VK_CALL(Device, vkCmdBindVertexBuffers(CommandBufferVk->Get(), Slot, 1, VertexBuffers, Offests));
}

void CCommandContextVulkan::SetPipeline(CCommandBuffer* CommandBuffer, CHandle<CPipelineState> Pipeline)
{
    m_ActivePipelineState = Pipeline;

    CASSERT(Pipeline.IsValid());
    if (Pipeline.IsValid() == false)
        return;
        
    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(CDeviceHandle::From(Pipeline));

    CPipelineStateVulkan* PipelineVk = BACKEND_DOWNCAST(Device.ResourceManager->GetPipelineState(Pipeline), CPipelineStateVulkan);
    CASSERT(PipelineVk->GetDesc().Type == PipelineType::Graphics);

    CCommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CCommandBufferVulkan);
    C_STATUS Result = PipelineVk->Bind(CommandBufferVk);

    C_ASSERT_RETURN(C_SUCCEEDED(Result));
}

C_STATUS CCommandContextVulkan::Init(IRendererBackend* Backend, CCommandBuffer* CommandBuffer)
{
    C_STATUS Result = CCommandContext::Init(Backend, CommandBuffer);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    m_BackendVk = GET_BACKEND_IMPL(Backend);
    m_CommandBufferVk = BACKEND_DOWNCAST(m_CommandBufferVk, CCommandBufferVulkan);

    return C_STATUS::C_STATUS_OK;
}

void CCommandContextVulkan::DeInit()
{
    DeInitImpl();
    CCommandContext::DeInit();
}

void CCommandContextVulkan::DeInitImpl()
{
    m_BackendVk = nullptr;
    m_CommandBufferVk = nullptr;

    m_ActivePipelineState = {};
}

void CCommandContextVulkan::OnBegin(CCommandBuffer* CommandBuffer)
{
    CCommandContext::OnBegin(CommandBuffer);

    CCommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CCommandBufferVulkan);
    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(CommandBufferVk->GetDeviceHandle());

    Device.BindlessManager->Bind(CommandBufferVk, PipelineType::Graphics);
    Device.BindlessManager->Bind(CommandBufferVk, PipelineType::Compute);
    // #todo_vk also add raytracing here?
}

void CCommandContextVulkan::OnEnd(CCommandBuffer* CommandBuffer)
{
    FLushPendingPipelineBarriers(CommandBuffer);
    CCommandContext::OnEnd(CommandBuffer);
}

void CCommandContextVulkan::InsertPipelineBarrier(CCommandBuffer* CommandBuffer, const CPipelineBarrier& Barrier, bool ForceFlush)
{
    CCommandContext::InsertPipelineBarrier(CommandBuffer, Barrier, ForceFlush);
}

void CCommandContextVulkan::FLushPendingPipelineBarriers(CCommandBuffer* CommandBuffer)
{
    // #tood_vk_barrier check for duplicates
    CCommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CCommandBufferVulkan);
    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(CommandBufferVk->GetDeviceHandle());

    C_STATUS Result = Device.UploadQueue->EnqueueGPUWork(CommandBuffer);
    CASSERT(C_SUCCEEDED(Result));

    if (m_PendingBarriers.empty() == false)
    {
        VkDependencyInfo BarrierDesc{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        BarrierDesc.dependencyFlags = 0;

        Vector<VkMemoryBarrier2> GlobalMemoryBarriers;
        Vector<VkBufferMemoryBarrier2> BufferMemoryBarriers;
        Vector<VkImageMemoryBarrier2> ImageMemoryBarriers;

        GlobalMemoryBarriers.reserve(m_PendingBarriers.size());
        BufferMemoryBarriers.reserve(m_PendingBarriers.size());
        ImageMemoryBarriers.reserve(m_PendingBarriers.size());

        for (uint32 i = 0; i < m_PendingBarriers.size(); ++i)
        {
            const auto& BarrierSrc = m_PendingBarriers[i];
            if (BarrierSrc.Action == EPipelineBarrierAction::GlobalMemory)
            {
                auto& Barrier = GlobalMemoryBarriers.emplace_back();
                Barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
                Barrier.srcStageMask = ConvertExecutionStageMask(BarrierSrc.SrcPipelineStageMask);
                Barrier.srcAccessMask = ConvertMemoryAccessMask(BarrierSrc.SrcMemoryAccessMask);
                Barrier.dstStageMask = ConvertExecutionStageMask(BarrierSrc.DstPipelineStageMask);
                Barrier.dstStageMask = ConvertMemoryAccessMask(BarrierSrc.DstMemoryAccessMask);
            }
            else if (BarrierSrc.Action == EPipelineBarrierAction::BufferMemory)
            {
                C_ASSERT_CONTINUE(BarrierSrc.Resource.IsValid());

                CResourceVulkan* ResourceVk = BACKEND_DOWNCAST(Device.ResourceManager->GetResource(BarrierSrc.Resource), CResourceVulkan);
                CASSERT(ResourceVk);
                CASSERT(ResourceVk->GetDesc().Flags & EResourceFlags::Buffer);

                auto& Barrier = BufferMemoryBarriers.emplace_back();
                Barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
                Barrier.buffer = ResourceVk->GetBuffer();
                Barrier.offset = BarrierSrc.Offset;
                Barrier.size = BarrierSrc.Size;
                Barrier.srcStageMask = ConvertExecutionStageMask(BarrierSrc.SrcPipelineStageMask);
                Barrier.srcAccessMask = ConvertMemoryAccessMask(BarrierSrc.SrcMemoryAccessMask);
                Barrier.dstStageMask = ConvertExecutionStageMask(BarrierSrc.DstPipelineStageMask);
                Barrier.dstAccessMask = ConvertMemoryAccessMask(BarrierSrc.DstMemoryAccessMask);
                Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // #todo_vk for queue transfership if needed
                Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            }
            else if (BarrierSrc.Action == EPipelineBarrierAction::TextureMemory)
            {
                C_ASSERT_CONTINUE(BarrierSrc.Resource.IsValid());

                CResourceVulkan* ResourceVk = BACKEND_DOWNCAST(Device.ResourceManager->GetResource(BarrierSrc.Resource), CResourceVulkan);
                CASSERT(ResourceVk);
                CASSERT(ResourceVk->GetDesc().Flags & EResourceFlags::Texture);

                auto& Barrier = ImageMemoryBarriers.emplace_back();
                Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                Barrier.image = ResourceVk->GetImage();
                Barrier.oldLayout = ConvertLayoutType(BarrierSrc.OldLayout);
                Barrier.newLayout = ConvertLayoutType(BarrierSrc.NewLayout);
                Barrier.subresourceRange.aspectMask = ConvertImageAspectType(BarrierSrc.SubresourceRange.AspectMask);
                Barrier.subresourceRange.baseMipLevel = BarrierSrc.SubresourceRange.BaseMipLevel;
                Barrier.subresourceRange.levelCount = BarrierSrc.SubresourceRange.LevelCount;
                Barrier.subresourceRange.baseArrayLayer = BarrierSrc.SubresourceRange.BaseArrayLayer;
                Barrier.subresourceRange.layerCount = BarrierSrc.SubresourceRange.LayerCount;
                Barrier.srcStageMask = ConvertExecutionStageMask(BarrierSrc.SrcPipelineStageMask);
                Barrier.srcAccessMask = ConvertMemoryAccessMask(BarrierSrc.SrcMemoryAccessMask);
                Barrier.dstStageMask = ConvertExecutionStageMask(BarrierSrc.DstPipelineStageMask);
                Barrier.dstAccessMask = ConvertMemoryAccessMask(BarrierSrc.DstMemoryAccessMask);
                Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // #todo_vk for queue transfership if needed
                Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            }
            else
            {
                CASSERT(false && "Barrier type not implemented");
            }
        }

        BarrierDesc.memoryBarrierCount = static_cast<uint32>(GlobalMemoryBarriers.size());
        BarrierDesc.pMemoryBarriers = GlobalMemoryBarriers.data();
        BarrierDesc.bufferMemoryBarrierCount = static_cast<uint32>(BufferMemoryBarriers.size());
        BarrierDesc.pBufferMemoryBarriers = BufferMemoryBarriers.data();
        BarrierDesc.imageMemoryBarrierCount = static_cast<uint32>(ImageMemoryBarriers.size());
        BarrierDesc.pImageMemoryBarriers = ImageMemoryBarriers.data();

        if (VK_CALL(Device, vkCmdPipelineBarrier2))
        {
            VK_CALL(Device, vkCmdPipelineBarrier2(CommandBufferVk->Get(), &BarrierDesc));
        }
        else if (VK_CALL(Device, vkCmdPipelineBarrier2KHR))
        {
            VK_CALL(Device, vkCmdPipelineBarrier2KHR(CommandBufferVk->Get(), &BarrierDesc));
        }
        else
        {
            CASSERT(false && "Vk: Synchronization2 extension issues");
        }
    }

    CCommandContext::FLushPendingPipelineBarriers(CommandBuffer);
}

} //namespace Cyclone::Render
