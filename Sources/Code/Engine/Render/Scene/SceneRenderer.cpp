#include "SceneRenderer.h"

#include "Engine/Framework/IRenderer.h"
#include "Engine/Framework/IApplication.h"

#include "RenderScene.h"
#include "RenderSceneView.h"
#include "RenderSceneSubsystem.h"

#include "Engine/Render/Backend/IRendererBackend.h"
#include "Engine/Render/Backend/IResourceManager.h"
#include "Engine/Render/Backend/WindowContext.h"
#include "Engine/Render/Backend/CommandQueue.h"
#include "Engine/Render/Backend/CommandBuffer.h"
#include "Engine/Render/Backend/Resource.h"
#include "Engine/Render/Backend/ResourceView.h"

namespace Cyclone::Render
{

// #todo_vk_refactor fixme 
extern void GUnitTestInit(IRendererBackend* Backend, CDeviceHandle Handle);
extern void GUnitTestDeInit(IRendererBackend* Backend);

CSceneRenderer::CSceneRenderer() = default;
CSceneRenderer::CSceneRenderer(CSceneRenderer&& Other) noexcept = default;
CSceneRenderer& CSceneRenderer::operator=(CSceneRenderer&& Other) noexcept = default;
CSceneRenderer::~CSceneRenderer()
{
    DeInitImpl();

    CASSERT(m_Renderer == nullptr);
}

C_STATUS CSceneRenderer::Init(CRenderSceneSubsystem* Subsystem, IRenderer* Renderer)
{
    m_Renderer = Renderer;
    m_Subsystem = Subsystem;

    return C_STATUS::C_STATUS_OK;
}

void CSceneRenderer::DeInit()
{
    DeInitImpl();
}

void CSceneRenderer::DeInitImpl() noexcept
{
    if (m_Renderer)
        GUnitTestDeInit(m_Renderer->GetRendererBackend());

    m_Renderer = nullptr;
    m_Subsystem = nullptr;
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
    for (uint32 s = 0; s < m_Subsystem->m_RenderScenes.size(); ++s)
    {
        auto& Scene = m_Subsystem->m_RenderScenes[s];
        for (uint32 SV = 0; SV < Scene->m_Views.size(); ++SV)
        {
            auto& SceneView = Scene->m_Views[SV];
            
            GUnitTestInit(m_Renderer->GetRendererBackend(), SceneView->RenderWindowContext->GetDeviceHandle());

            //SceneView->BeginRender();
        }
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CSceneRenderer::Render()
{
    // #todo_vk #todo_mt test multithreading here

    for (uint32 s = 0; s < m_Subsystem->m_RenderScenes.size(); ++s)
    {
        auto& Scene = m_Subsystem->m_RenderScenes[s];
        for (uint32 i = 0; i < Scene->m_Views.size(); ++i)
        {
            auto& SceneView = Scene->m_Views[i];

            RenderSceneView(SceneView.get(), i);
        }
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CSceneRenderer::EndRender()
{
    for (uint32 s = 0; s < m_Subsystem->m_RenderScenes.size(); ++s)
    {
        auto& Scene = m_Subsystem->m_RenderScenes[s];
        for (uint32 SV = 0; SV < Scene->m_Views.size(); ++SV)
        {
            auto& SceneView = Scene->m_Views[SV];

            //SceneView->BeginRender();
        }
    }

    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone::Render
