#include "CommandQueue.h"

namespace Cyclone::Render
{

CCommandQueue::CCommandQueue() = default;
CCommandQueue::~CCommandQueue() = default;

C_STATUS CCommandQueue::Init(CommandQueueDesc Desc)
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CCommandQueue::OnBeginRender()
{
    return C_STATUS::C_STATUS_OK;

}

C_STATUS CCommandQueue::OnEndRender()
{
    return C_STATUS::C_STATUS_OK;

}

CCommandBuffer* CCommandQueue::AllocateCommandBuffer()
{
    return nullptr;
}

void CCommandQueue::ReturnCommandBuffer(CCommandBuffer* CommandBuffer)
{

}

C_STATUS CCommandQueue::Submit(CCommandBuffer** CommandBuffers, uint32_t CommandBuffersCount, bool AutoReturnToPool /*= true*/)
{
    return C_STATUS::C_STATUS_OK;
}

void CCommandQueue::DeInit()
{

}

} // Cyclone::Render
