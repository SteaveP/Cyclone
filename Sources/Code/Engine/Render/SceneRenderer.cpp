#include "SceneRenderer.h"

#include "Engine/Framework/IRenderer.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneViewport.h"

#include "IRendererBackend.h"
#include "RenderScene.h"
#include "RenderSceneView.h"

#include "Types/WindowContext.h"
#include "Types/CommandQueue.h"
#include "Types/CommandBuffer.h"

namespace Cyclone::Render
{

CSceneRenderer& CSceneRenderer::operator=(CSceneRenderer&& Other) = default;
CSceneRenderer::CSceneRenderer(CSceneRenderer && Other) = default;

CSceneRenderer::CSceneRenderer() = default;
CSceneRenderer::~CSceneRenderer()
{
    CASSERT(m_Renderer == nullptr);
}

C_STATUS CSceneRenderer::Init(IRenderer* Renderer)
{
    m_Renderer = Renderer;

    return C_STATUS::C_STATUS_OK;
}

void CSceneRenderer::DeInit()
{
    //for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
    //{
    //    if (auto& WindowContext = m_WindowContexts[i])
    //        WindowContext->Shutdown();
    //}

    m_RenderScenes.clear();
    m_Renderer = nullptr;
}

C_STATUS CSceneRenderer::BeginFrame()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CSceneRenderer::EndFrame()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CSceneRenderer::BeginRender()
{
    for (uint32 s = 0; s < m_RenderScenes.size(); ++s)
    {
        auto& Scene = m_RenderScenes[s];
        for (uint32 SV = 0; SV < Scene->m_Views.size(); ++SV)
        {
            auto& SceneView = Scene->m_Views[SV];

            //SceneView->BeginRender();
        }
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CSceneRenderer::Render()
{
    // #todo_vk test multithreading here

    for (uint32 s = 0; s < m_RenderScenes.size(); ++s)
    {
        auto& Scene = m_RenderScenes[s];
        for (uint32 i = 0; i < Scene->m_Views.size(); ++i)
        {
            auto& SceneView = Scene->m_Views[i];

            RenderSceneView(SceneView.get());
        }
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CSceneRenderer::EndRender()
{
    for (uint32 s = 0; s < m_RenderScenes.size(); ++s)
    {
        auto& Scene = m_RenderScenes[s];
        for (uint32 SV = 0; SV < Scene->m_Views.size(); ++SV)
        {
            auto& SceneView = Scene->m_Views[SV];

            //SceneView->BeginRender();
        }
    }

    return C_STATUS::C_STATUS_OK;
}

CRenderScene* CSceneRenderer::AddScene(CScene* Scene)
{
    CASSERT(GetRenderScene(Scene) == nullptr);

    auto& RenderScene = m_RenderScenes.emplace_back(MakeUnique<CRenderScene>());
    RenderScene->m_Scene = Scene;

    return RenderScene.get();
}

void CSceneRenderer::RemoveScene(CScene* Scene)
{
    CASSERT(false);
}

CRenderSceneView* CSceneRenderer::AddViewport(CSceneViewport* Viewport)
{
    // #todo_vk check for duplicates
    
    auto RenderSceneView = MakeShared<CRenderSceneView>();

    const auto& RenderScene = GetRenderScene(Viewport->Scene.get());

    RenderSceneView->Viewport = Viewport;
    RenderSceneView->RenderScene = RenderScene;
    RenderSceneView->RenderWindowContext = m_Renderer->GetWindowContext(Viewport->Window.get());

    C_STATUS Result = RenderScene->AddSceneView(RenderSceneView);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), nullptr);

    return nullptr;
}

void CSceneRenderer::RemoveViewport(CSceneViewport* Viewport)
{
    CASSERT(false);
}

CRenderScene* CSceneRenderer::GetRenderScene(CScene* Scene)
{
    for (uint32 s = 0; s < m_RenderScenes.size(); ++s)
    {
        if(m_RenderScenes[s] && m_RenderScenes[s]->m_Scene == Scene)
            return m_RenderScenes[s].get();
    }

    return nullptr;
}

} // namespace Cyclone::Render
