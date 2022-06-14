#include "RenderBackendDX12.h"

#include "CommonDX12.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"

namespace Cyclone::Render
{

C_STATUS RenderBackendDX12::Init(IRenderer* Renderer)
{
    m_Renderer = Renderer;

    C_STATUS Result = C_STATUS::C_STATUS_OK;
    {

    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendDX12::Shutdown()
{
    // #todo_dx12 wait for GPU?
    //m_GlobalContext.Shutdown();

    return C_STATUS::C_STATUS_OK;
}


C_STATUS RenderBackendDX12::BeginRender()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendDX12::EndRender()
{
    return C_STATUS::C_STATUS_OK;
}

WindowContext* RenderBackendDX12::CreateWindowContext(IWindow* Window)
{
    return nullptr;
}

CCommandQueue* RenderBackendDX12::CreateCommandQueue()
{
    return nullptr;

}

CCommandBuffer* RenderBackendDX12::CreateCommandBuffer()
{
    return nullptr;

}

CTexture* RenderBackendDX12::CreateTexture()
{
    return nullptr;
}

} // namespace Cyclone::Render
