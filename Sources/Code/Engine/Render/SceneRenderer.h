#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

#include "Common.h"

namespace Cyclone
{
    class IRenderer;
    class CScene;
    class CSceneViewport;
}

namespace Cyclone::Render
{

class ENGINE_API CSceneRenderer
{
public:
    DISABLE_COPY(CSceneRenderer);

    CSceneRenderer(CSceneRenderer&& Other);
    CSceneRenderer& operator =(CSceneRenderer && Other);

    CSceneRenderer();
    virtual ~CSceneRenderer();

    virtual C_STATUS Init(IRenderer* Renderer);
    virtual void DeInit();

    virtual C_STATUS BeginFrame();
    virtual C_STATUS EndFrame();

    virtual C_STATUS BeginRender();
    virtual C_STATUS Render();
    virtual C_STATUS EndRender();

    virtual CRenderScene* AddScene(CScene* Scene);
    virtual void RemoveScene(CScene* Scene);

    virtual CRenderScene* GetRenderScene(CScene* Scene);

    virtual CRenderSceneView* AddViewport(CSceneViewport* Viewport);
    virtual void RemoveViewport(CSceneViewport* Viewport);

    void RenderSceneView(CRenderSceneView* SceneView);

protected:
    Vector<UniquePtr<CRenderScene>> m_RenderScenes;
    IRenderer* m_Renderer = nullptr;
};

} // namespace Cyclone::Render
