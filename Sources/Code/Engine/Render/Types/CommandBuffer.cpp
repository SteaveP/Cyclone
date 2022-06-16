#include "CommandBuffer.h"

namespace Cyclone::Render
{

CCommandBuffer::CCommandBuffer() = default;
CCommandBuffer::~CCommandBuffer() = default;

void CCommandBuffer::DeInit()
{
    m_Context.reset();
}

C_STATUS CCommandBuffer::Begin()
{
    return C_STATUS::C_STATUS_OK;
}

void CCommandBuffer::End()
{

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

void CCommandBuffer::Copy()
{

}

void CCommandBuffer::Clear()
{

}

Ptr<CPipeline> CCommandBuffer::SetPipeline(const CPipelineDesc& Desc)
{
    return m_Context->SetPipeline(this, Desc);
}

void CCommandBuffer::SetIndexBuffer(CBuffer* IndexBuffer, uint32 Offset)
{
    return m_Context->SetIndexBuffer(this, IndexBuffer, Offset);
}

void CCommandBuffer::SetVertexBuffer(CBuffer* VertexBuffer, uint32 Slot, uint64 Offset)
{
    return m_Context->SetVertexBuffer(this, VertexBuffer, Slot, Offset);
}

C_STATUS CCommandContext::Init(IRenderer* Renderer, CCommandBuffer* CommandBuffer)
{
    m_Renderer = Renderer;
    m_CommandBuffer = CommandBuffer;
    return C_STATUS::C_STATUS_OK;
}

void CCommandContext::DeInit()
{

}

void CCommandContext::BeginRenderPass(CCommandBuffer* CommandBuffer, const CRenderPass& RenderPass)
{
    m_RenderPass = RenderPass;
}

void CCommandContext::EndRenderPass(CCommandBuffer* CommandBuffer)
{

}

Ptr<CPipeline> CCommandContext::SetPipeline(CCommandBuffer* CommandBuffer, const CPipelineDesc& Desc)
{
    CASSERT(false);
    return nullptr;
}

} // namespace Cyclone::Render
