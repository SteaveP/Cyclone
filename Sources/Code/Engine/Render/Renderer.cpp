#include "Renderer.h"

namespace Cyclone::Render
{

C_STATUS Renderer::Init(const RendererDesc* desc)
{
    CASSERT(m_rendererBackend);

    if (m_rendererBackend)
    {
        C_STATUS result = m_rendererBackend->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    return C_STATUS::C_STATUS_OK;
}

void Renderer::Deinit()
{
        
}

C_STATUS Renderer::BeginFrame()
{
        
    return C_STATUS::C_STATUS_OK;
}

C_STATUS Renderer::Render()
{
    if (m_rendererBackend)
    {
        C_STATUS result = m_rendererBackend->Render();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    return C_STATUS::C_STATUS_OK;
}

void Renderer::OnResize(const IWindow* window)
{
        
}

void Renderer::WaitGPU()
{
        
}

void Renderer::SetSceneRenderer(ISceneRenderer* sceneRenderer)
{
        
}

ISceneRenderer* Renderer::GetSceneRenderer() const
{
        
    return nullptr;
}

UIRenderer* Renderer::GetUIRenderer() const
{
        
    return nullptr;
}

C_STATUS Renderer::InitConcrete(IRendererBackend* RendererBackend)
{
    m_rendererBackend = RendererBackend;
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone::Render
