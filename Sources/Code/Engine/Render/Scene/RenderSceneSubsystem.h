#pragma once

#include "Engine/Framework/ISubsystem.h"
#include "Engine/Utils/Delegate.h"

#include "Engine/Render/CommonRender.h"

namespace Cyclone
{
    class IRenderer;
    class CScene;
    class CSceneViewport;
    class CSceneSubsystem;
}

namespace Cyclone::Render
{

class CSceneRenderer;
class CRenderScene;
class CRenderSceneView;

class ENGINE_API CRenderSceneSubsystem : public ISubsystem
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CRenderSceneSubsystem);

    CRenderSceneSubsystem();
    virtual ~CRenderSceneSubsystem();

    virtual C_STATUS Init(IApplication* App) override;
    virtual void DeInit();

    virtual C_STATUS OnRegister() override;
    virtual void OnUnRegister() override;

    CSceneRenderer* GetSceneRenderer() { return m_SceneRenderer.get(); }

    virtual Ptr<CRenderScene> GetRenderScene(CScene* Scene);

    virtual Ptr<CRenderSceneView> GetRenderSceneView(CSceneViewport* Viewport);

public:
    using CRenderSceneDelegate = DelegateLib::MulticastDelegate1<Ptr<CRenderScene>>;
    using CRenderSceneViewDelegate = DelegateLib::MulticastDelegate1<Ptr<CRenderSceneView>>;

    CRenderSceneDelegate OnRenderSceneAddedDelegate;
    CRenderSceneDelegate OnRenderSceneRemovedDelegate;

    CRenderSceneViewDelegate OnRenderSceneViewAddedDelegate;
    CRenderSceneViewDelegate OnRenderSceneViewRemovedDelegate;

protected:
    void OnSceneAdded(Ptr<CScene> Scene);
    void OnSceneRemoved(Ptr<CScene> Scene);

    void OnSceneViewAdded(Ptr<CSceneViewport> SceneView);
    void OnSceneViewRemoved(Ptr<CSceneViewport> SceneView);

    void OnBeginRender();
    void OnRender();
    void OnEndRender();

private:
    void DeInitImpl() noexcept;

protected:
    Vector<Ptr<CRenderScene>> m_RenderScenes;
    UniquePtr<CSceneRenderer> m_SceneRenderer;

    CSceneSubsystem* m_SceneSys = nullptr;
    IRenderer* m_Renderer = nullptr;
    IApplication* m_App = nullptr;
    
    friend class CSceneRenderer;
};

} // namespace Cyclone::Render
