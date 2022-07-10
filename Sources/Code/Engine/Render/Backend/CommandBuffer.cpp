#include "CommandBuffer.h"

#include "Engine/Render/Backend/IRendererBackend.h"
#include "Engine/Render/Backend/IResourceManager.h"
#include "Resource.h"

#include "Barrier.h"

namespace Cyclone::Render
{

CCommandBuffer::CCommandBuffer() = default;
CCommandBuffer::~CCommandBuffer()
{
    DeInitImpl();
}

CCommandContext::CCommandContext() = default;
CCommandContext::CCommandContext(CCommandContext&& Other) noexcept = default;
CCommandContext& CCommandContext::operator =(CCommandContext&& Other) noexcept = default;
CCommandContext::~CCommandContext()
{
    DeInitImpl();
}

void CCommandBuffer::DeInit()
{
    DeInitImpl();
}

void CCommandBuffer::DeInitImpl() noexcept
{
    m_Context.reset();
}

void CCommandContext::DeInit()
{
    DeInitImpl();
}

void CCommandContext::DeInitImpl() noexcept
{
    CASSERT(m_PendingBarriers.empty());
}


C_STATUS CCommandBuffer::Begin()
{
    m_Context->OnBegin(this);
    return C_STATUS::C_STATUS_OK;
}

void CCommandBuffer::End()
{
    m_Context->OnEnd(this);
}

void CCommandBuffer::SetRenderTargets(CRenderTargetSet RenderTargetSet)
{
    CASSERT(false);
    //m_Context->SetRenderTargets(MoveTemp(RenderTargetSet));
}

void CCommandBuffer::BeginRenderPass(const CRenderPass& RenderPass)
{
    m_Context->BeginRenderPass(this, RenderPass);
}

void CCommandBuffer::EndRenderPass()
{
    m_Context->EndRenderPass(this);
}

void CCommandBuffer::Draw(uint32 IndexCount, uint32 InstanceCount, uint32 FirstIndex, int32 VertexOffset, uint32 FirstInstance)
{

}

void CCommandBuffer::Dispatch()
{

}

void CCommandBuffer::Clear()
{

}

void CCommandBuffer::SetPipeline(CHandle<CPipelineState> Pipeline)
{
    return m_Context->SetPipeline(this, Pipeline);
}

void CCommandBuffer::SetIndexBuffer(CHandle<CResource> IndexBuffer, uint64 Offset, EFormatType Format)
{
    return m_Context->SetIndexBuffer(this, IndexBuffer, Offset, Format);
}

void CCommandBuffer::SetVertexBuffer(CHandle<CResource> VertexBuffer, uint32 Slot, uint64 Offset)
{
    return m_Context->SetVertexBuffer(this, VertexBuffer, Slot, Offset);
}

void CCommandBuffer::InsertPipelineBarrier(CPipelineBarrier Barrier, bool ForceFlush)
{
    m_Context->InsertPipelineBarrier(this, Barrier, ForceFlush);
}

void CCommandBuffer::FLushPendingPipelineBarriers()
{
    m_Context->FLushPendingPipelineBarriers(this);
}

void CCommandBuffer::CopyBuffer(const CCopyBufferDesc& Desc)
{
    FLushPendingPipelineBarriers();
}

void CCommandBuffer::CopyBufferToTexture(const CCopyBufferToTextureDesc& Desc)
{
    FLushPendingPipelineBarriers();
}

C_STATUS CCommandContext::Init(IRendererBackend* Backend, CCommandBuffer* CommandBuffer)
{
    m_Backend = Backend;
    m_CommandBuffer = CommandBuffer;
    return C_STATUS::C_STATUS_OK;
}

void CCommandContext::BeginRenderPass(CCommandBuffer* CommandBuffer, const CRenderPass& RenderPass)
{
    // #todo_vk validate that there was no active render pass
    m_ActiveRenderPass = RenderPass;

    IResourceManager* ResourceManager = m_Backend->GetResourceManager(CommandBuffer->GetDeviceHandle());
    CASSERT(ResourceManager);

    for (uint32 i = 0; i < RenderPass.RenderTargetSet.RenderTargetsCount; ++i)
    {
        const auto& RT = RenderPass.RenderTargetSet.RenderTargets[i];
        CPipelineBarrier Barrier = CPipelineBarrier::FromTextureAuto(m_Backend, RT.RenderTarget.Texture, EImageLayoutType::ColorAttachment, EImageUsageType::ColorAttachment, RT.LoadOp == ERenderTargetLoadOp::Load);
        CommandBuffer->InsertPipelineBarrier(MoveTemp(Barrier));

    }
    if (RenderPass.RenderTargetSet.DepthScentil.IsValid())
    {
        const auto& DT = RenderPass.RenderTargetSet.DepthScentil;

        // #todo_vk_depth here check if can be set depthStencil read only optimal state
        CPipelineBarrier Barrier = CPipelineBarrier::FromTextureAuto(m_Backend, DT.RenderTarget.Texture, EImageLayoutType::DepthStencil, EImageUsageType::DepthStencil, DT.LoadOp == ERenderTargetLoadOp::Load);
        CommandBuffer->InsertPipelineBarrier(MoveTemp(Barrier));
    }

    FLushPendingPipelineBarriers(CommandBuffer);

}

void CCommandContext::EndRenderPass(CCommandBuffer* CommandBuffer)
{
    // #todo_vk validate that there was active render pass
    IResourceManager* ResourceManager = m_Backend->GetResourceManager(CommandBuffer->GetDeviceHandle());
    CASSERT(ResourceManager);

    for (uint32 i = 0; i < m_ActiveRenderPass.RenderTargetSet.RenderTargetsCount; ++i)
    {
        const auto& RT = m_ActiveRenderPass.RenderTargetSet.RenderTargets[i];
        if (RT.StoreOp == ERenderTargetStoreOp::Store)
        {
            auto Barrier = CPipelineBarrier::FromTextureAuto(m_Backend, RT.RenderTarget.Texture, RT.FinalLayout, RT.FinalUsage, true);
            CommandBuffer->InsertPipelineBarrier(MoveTemp(Barrier));
        }
    }
    if (m_ActiveRenderPass.RenderTargetSet.DepthScentil.IsValid())
    {
        const auto& DT = m_ActiveRenderPass.RenderTargetSet.DepthScentil;
        
        if (DT.StoreOp == ERenderTargetStoreOp::Store)
        {
            auto Barrier = CPipelineBarrier::FromTextureAuto(m_Backend, DT.RenderTarget.Texture, DT.FinalLayout, DT.FinalUsage, true);
            CommandBuffer->InsertPipelineBarrier(MoveTemp(Barrier));
        }
    }

    m_ActiveRenderPass = {};
}

void CCommandContext::SetPipeline(CCommandBuffer* CommandBuffer, CHandle<CPipelineState> Pipeline)
{
    CASSERT(false);
}

void CCommandContext::OnBegin(CCommandBuffer* CommandBuffer)
{
    CASSERT(m_PendingBarriers.empty());
}

void CCommandContext::OnEnd(CCommandBuffer* CommandBuffer)
{
    // flush barriers should be done in overridden methods
    // #todo_vk preprocess barriers to remove redundant transitions
    CASSERT(m_PendingBarriers.empty());
}

void CCommandContext::InsertPipelineBarrier(CCommandBuffer* CommandBuffer, const CPipelineBarrier& Barrier, bool ForceFlush)
{
    m_PendingBarriers.emplace_back(MoveTemp(Barrier));

    if (Barrier.Action == EPipelineBarrierAction::BufferMemory || Barrier.Action == EPipelineBarrierAction::TextureMemory)
    {
        CASSERT(Barrier.Resource.IsValid());

        IResourceManager* ResourceManager = m_Backend->GetResourceManager(CDeviceHandle::From(Barrier.Resource));
        CASSERT(ResourceManager);

        CResource* ResourcePtr = ResourceManager->GetResource(Barrier.Resource);
        CASSERT(ResourcePtr);
        if (ResourcePtr)
        {
            ResourcePtr->UpdateTraceableLayout(Barrier.NewLayout);
            ResourcePtr->UpdateTraceableUsageType(Barrier.NewUsage);
        }
    }

    if (ForceFlush)
    {
        FLushPendingPipelineBarriers(CommandBuffer);
    }
}

void CCommandContext::FLushPendingPipelineBarriers(CCommandBuffer* CommandBuffer)
{
    m_PendingBarriers.clear();
}

} // namespace Cyclone::Render
