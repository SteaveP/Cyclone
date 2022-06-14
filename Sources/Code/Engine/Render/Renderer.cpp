#include "Renderer.h"

#include "Engine/Framework/IApplication.h"
#include "Types/WindowContext.h"
#include "RenderScene.h"
#include "RenderSceneView.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneViewport.h"

namespace Cyclone::Render
{

Renderer::Renderer() = default;
Renderer::~Renderer()
{
    CASSERT(m_RendererBackend == nullptr);
}

C_STATUS Renderer::Init(const RendererDesc* Desc)
{
    CASSERT(m_RendererBackend);

    m_App = Desc->App;
    m_Windows = Desc->Windows;

    if (m_RendererBackend)
    {
        C_STATUS Result = m_RendererBackend->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    for (uint32 i = 0; i < m_Windows.size(); ++i)
    {
        if (auto Window = m_Windows[i])
        {
            OnAddWindow(Window);
        }
    }

    m_RenderScene = std::make_unique<CRenderScene>();

    return C_STATUS::C_STATUS_OK;
}

void Renderer::Deinit()
{
    for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
    {
        if (auto& WindowContext = m_WindowContexts[i])
            WindowContext->Shutdown();
    }
    m_WindowContexts.clear();

    if (m_RendererBackend)
    {
        m_RendererBackend->Shutdown();
        m_RendererBackend = nullptr;
    }
}

C_STATUS Renderer::BeginFrame()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS Renderer::EndFrame()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS Renderer::BeginRender()
{
    if (m_RendererBackend)
    {
        C_STATUS result = m_RendererBackend->BeginRender();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
    {
        C_STATUS Result = m_WindowContexts[i]->BeginRender();
        if (!C_SUCCEEDED(Result))
            return Result;
    }

    auto& Scene = m_RenderScene;
    for (uint32 SV = 0; SV < Scene->m_Views.size(); ++SV)
    {
        auto& SceneView = Scene->m_Views[SV];

        //SceneView->BeginRender();
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS Renderer::Render()
{
    // #todo_vk test multithreading here and mult-scene support
    Array<CRenderScene*, 1> Scenes = { m_RenderScene.get() };
    for (uint32 s = 0; s < Scenes.size(); ++s)
    {
        auto& Scene = m_RenderScene;
        for (uint32 i = 0; i < Scene->m_Views.size(); ++i)
        {
            auto& SceneView = Scene->m_Views[i];

            RenderSceneView(SceneView.get());
        }
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS Renderer::EndRender()
{
    auto& Scene = m_RenderScene;
    for (uint32 SV = 0; SV < Scene->m_Views.size(); ++SV)
    {
        auto& SceneView = Scene->m_Views[SV];

        //SceneView->EndRender();
    }

    // present
    for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
    {
        C_STATUS Result = m_WindowContexts[i]->Present();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    if (m_RendererBackend)
    {
        C_STATUS result = m_RendererBackend->EndRender();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    m_CurrentFrame++;
    m_CurrentLocalFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return C_STATUS::C_STATUS_OK;
}

void Renderer::WaitGPU()
{
    m_RendererBackend->WaitGPU();
}

C_STATUS Renderer::PreInit(IRendererBackend* RendererBackend)
{
    m_RendererBackend = RendererBackend;
    return C_STATUS::C_STATUS_OK;
}

CWindowContext* Renderer::GetWindowContext(IWindow* Window)
{
    for (auto& Context : m_WindowContexts)
    {
        if (Context->GetWindow() == Window)
            return Context.get();
    }
    return nullptr;
}

CWindowContext* Renderer::GetDefaultWindowContext()
{
    if (m_WindowContexts.size() > 0)
        return m_WindowContexts[0].get();

    return nullptr;
}

CCommandQueue* Renderer::GetDefaultCommandQueue(CommandQueueType Type)
{
    if (CWindowContext* WindowContext = GetDefaultWindowContext())
        return WindowContext->GetCommandQueue(Type);

    return nullptr;
}

CWindowContext* Renderer::OnAddWindow(IWindow* Window)
{
    if (Window == nullptr)
        return nullptr;

#if _DEBUG
    CASSERT(GetWindowContext(Window) == nullptr);
#endif
    
    m_WindowContexts.emplace_back(m_RendererBackend->CreateWindowContext(Window));
    C_ASSERT_RETURN_VAL(m_WindowContexts.back(), nullptr);

    return nullptr;
}

void Renderer::OnRemoveWindow(IWindow* Window)
{
#if _DEBUG
    CASSERT(GetWindowContext(Window) != nullptr);
#endif

    // #todo_vk remove scene views for each of this window?

    for(auto it = m_WindowContexts.begin(); it != m_WindowContexts.end(); ++it)
    {
        if ((*it)->GetWindow() == Window)
        {
            m_WindowContexts.erase(it);
            return;
        }
    }
}

CRenderScene* Renderer::AddScene(CScene* Scene)
{
    CASSERT(m_RenderScene);
    m_RenderScene->m_Scene = Scene;

    return m_RenderScene.get();
}

void Renderer::RemoveScene(CScene* Scene)
{
    CASSERT(false);
}

CRenderSceneView* Renderer::AddViewport(CSceneViewport* Viewport)
{
    // #todo_vk check for duplicates
    
    auto& RenderSceneView = std::make_shared<CRenderSceneView>();

    const auto& RenderScene = GetRenderScene(Viewport->Scene.get());

    RenderSceneView->Viewport = Viewport;
    RenderSceneView->RenderScene = RenderScene;
    RenderSceneView->RenderWindowContext = GetWindowContext(Viewport->Window.get());

    C_STATUS Result = RenderScene->AddSceneView(std::move(RenderSceneView));
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), nullptr);

    return nullptr;
}

void Renderer::RemoveViewport(CSceneViewport* Viewport)
{
    CASSERT(false);
}

CRenderScene* Renderer::GetRenderScene(CScene* Scene)
{
    if (m_RenderScene && m_RenderScene->m_Scene == Scene)
        return m_RenderScene.get();

    return nullptr;
}

} // namespace Cyclone::Render
