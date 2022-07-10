#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

#include "Engine/Render/CommonRender.h"

namespace Cyclone
{
    class IRenderer;
    class CScene;
    class CSceneViewport;
}

namespace Cyclone::Render
{

class CRenderSceneSubsystem;
class CRenderScene;
class CRenderSceneView;

class ENGINE_API CSceneRenderer
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CSceneRenderer);

    CSceneRenderer();
    virtual ~CSceneRenderer();

    virtual C_STATUS Init(CRenderSceneSubsystem* Subsystem, IRenderer* Renderer);
    virtual void DeInit();

    virtual C_STATUS BeginFrame();
    virtual C_STATUS EndFrame();

    virtual C_STATUS BeginRender();
    virtual C_STATUS Render();
    virtual C_STATUS EndRender();

    void RenderSceneView(CRenderSceneView* SceneView, uint32 SceneViewIndex);

private:
    void DeInitImpl() noexcept;

protected:
    CRenderSceneSubsystem* m_Subsystem = nullptr;
    IRenderer* m_Renderer = nullptr;
};

} // namespace Cyclone::Render
