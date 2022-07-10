#pragma once

#include "Engine/Framework/ISubsystem.h"
#include "Engine/Utils/Delegate.h"

namespace Cyclone
{

class IWindow;
class CScene;
class CSceneViewport;

class ENGINE_API CSceneSubsystem : public ISubsystem
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CSceneSubsystem);

    CSceneSubsystem();
    virtual ~CSceneSubsystem();

    virtual C_STATUS Init(IApplication* App) override;
    virtual void DeInit() override;

    virtual C_STATUS OnRegister() override;
    virtual void OnUnRegister() override;

    // Scenes
    Ptr<CScene> AddScene(String Name);
    void RemoveScene(Ptr<CScene> Scene);
    void RemoveScene(const String& Name);

    Ptr<CScene> GetScene(const String& Name);
    Ptr<CScene> GetScene(uint32 Index) { return m_Scenes[Index]; }
    uint32 GetSceneCount() const { return static_cast<uint32>(m_Scenes.size()); }

    // Viewports
    Ptr<CSceneViewport> AddViewport(String Name, Ptr<CScene> Scene, Ptr<IWindow> Window);
    void RemoveViewport(const String& Name);
    void RemoveViewport(Ptr<CSceneViewport> Viewport);

    Ptr<CSceneViewport> GetViewport(const String& Name);
    Ptr<CSceneViewport> GetViewport(uint32 Index) { return m_Viewports[Index]; }
    uint32 GetViewportCount() const { return static_cast<uint32>(m_Viewports.size()); }

public:
    using CSceneDelegate = DelegateLib::MulticastDelegate1<Ptr<CScene>>;
    using CSceneViewportDelegate = DelegateLib::MulticastDelegate1<Ptr<CSceneViewport>>;

    CSceneDelegate OnSceneAddedDelegate;
    CSceneDelegate OnSceneRemovedDelegate;

    CSceneViewportDelegate OnSceneViewportAddedDelegate;
    CSceneViewportDelegate OnSceneViewportRemovedDelegate;

private:
    void DeInitImpl() noexcept;

protected:
    IApplication* m_App = nullptr;

    Vector<Ptr<CScene>> m_Scenes;
    Vector<Ptr<CSceneViewport>> m_Viewports;

};

}
