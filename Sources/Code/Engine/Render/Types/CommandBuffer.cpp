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
    //m_Context->SetRenderTargets(std::move(RenderTargetSet));
}

void CCommandBuffer::BeginRenderPass(const CRenderPass& RenderPass)
{
    m_Context->BeginRenderPass(this, RenderPass);
}

void CCommandBuffer::EndRenderPass()
{
    m_Context->EndRenderPass(this);
}

void CCommandBuffer::Draw()
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

} // namespace Cyclone::Render
