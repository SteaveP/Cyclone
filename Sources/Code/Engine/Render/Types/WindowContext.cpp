#include "WindowContext.h"

#include "Engine/Core/Helpers.h"

#include "Engine/Framework/IRenderer.h"
#include "Engine/Framework/IWindow.h"
#include "Engine/Render/IRendererBackend.h"

namespace Cyclone::Render
{

CWindowContext::CWindowContext() = default;
CWindowContext::~CWindowContext() {}

C_STATUS CWindowContext::Init(IRenderer* Renderer, IWindow* Window)
{
    m_Renderer = Renderer;
    m_Window = Window;

    if (m_Renderer == nullptr || m_Window == nullptr)
        return C_STATUS::C_STATUS_INVALID_ARG;

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CWindowContext::Shutdown()
{
    m_Renderer = nullptr;
    m_Window = nullptr;
    m_BackBuffers.clear();

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CWindowContext::BeginRender()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CWindowContext::Present()
{
    return C_STATUS::C_STATUS_OK;
}

}
