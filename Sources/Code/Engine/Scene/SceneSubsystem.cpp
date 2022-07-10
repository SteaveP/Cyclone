#include "SceneSubsystem.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneViewport.h"

#include "Engine/Framework/IWindow.h"

namespace Cyclone
{

CSceneSubsystem::CSceneSubsystem() : ISubsystem("SceneSubsystem") {}
CSceneSubsystem::CSceneSubsystem(CSceneSubsystem&& Other) noexcept = default;
CSceneSubsystem& CSceneSubsystem::operator =(CSceneSubsystem&& Other) noexcept = default;
CSceneSubsystem::~CSceneSubsystem()
{
    DeInitImpl();
}

C_STATUS CSceneSubsystem::Init(IApplication* App)
{
    m_App = App;
    return C_STATUS::C_STATUS_OK;
}

void CSceneSubsystem::DeInit()
{
    DeInitImpl();
}

void CSceneSubsystem::DeInitImpl() noexcept
{
    CASSERT(m_Scenes.empty());
    CASSERT(m_Viewports.empty());

    m_App = nullptr;
}

C_STATUS CSceneSubsystem::OnRegister()
{
    return C_STATUS::C_STATUS_OK;
}

void CSceneSubsystem::OnUnRegister()
{
    CASSERT(m_Scenes.empty());
    CASSERT(m_Viewports.empty());
}

Ptr<CScene> CSceneSubsystem::AddScene(String Name)
{
    auto Scene = m_Scenes.emplace_back(MakeShared<CScene>());
    Scene->m_Name = Name;

    if (OnSceneAddedDelegate)
    {
        OnSceneAddedDelegate(Scene);
    }

    return Scene;
}

void CSceneSubsystem::RemoveScene(Ptr<CScene> Scene)
{
    // Remove viewports first
    for (auto it = m_Viewports.begin(); it != m_Viewports.end();)
    {
        if ((*it)->Scene == Scene)
        {
            if (OnSceneViewportRemovedDelegate)
                OnSceneViewportRemovedDelegate(*it);

            it = m_Viewports.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Remove scene
    for (auto it = m_Scenes.begin(); it != m_Scenes.end(); ++it)
    {
        if (*it == Scene)
        {
            if (OnSceneRemovedDelegate)
                OnSceneRemovedDelegate(*it);

            m_Scenes.erase(it);
            return;
        }
    }
}

void CSceneSubsystem::RemoveScene(const String& Name)
{
    auto Scene = GetScene(Name);
    RemoveScene(Scene);
}

Ptr<CScene> CSceneSubsystem::GetScene(const String& Name)
{
    for (auto it = m_Scenes.begin(); it != m_Scenes.end(); ++it)
    {
        if ((*it)->GetName() == Name)
        {
            return *it;
        }
    }

    return nullptr;
}

Ptr<CSceneViewport> CSceneSubsystem::AddViewport(String Name, Ptr<CScene> Scene, Ptr<IWindow> Window)
{
    auto Viewport = m_Viewports.emplace_back(MakeShared<CSceneViewport>());
    Viewport->m_Name = Name;
    Viewport->Window = Window;
    Viewport->Scene = Scene;
    Viewport->UpperLeftCorner = Vec2{ 0.f, 0.f };
    Viewport->BottomRightCorner = Vec2{ (float)Window->GetWidth(), (float)Window->GetHeight() };
    Viewport->Camera = MakeShared<CCamera>();

    if (OnSceneViewportAddedDelegate)
        OnSceneViewportAddedDelegate(Viewport);

    return Viewport;
}

void CSceneSubsystem::RemoveViewport(const String& Name)
{
    auto Viewport = GetViewport(Name);
    RemoveViewport(Viewport);
}

void CSceneSubsystem::RemoveViewport(Ptr<CSceneViewport> Viewport)
{
    for (auto it = m_Viewports.begin(); it != m_Viewports.end(); ++it)
    {
        if (*it == Viewport)
        {
            if (OnSceneViewportRemovedDelegate)
                OnSceneViewportRemovedDelegate(*it);

            m_Viewports.erase(it);
            return;
        }
    }
}

Ptr<CSceneViewport> CSceneSubsystem::GetViewport(const String& Name)
{
    for (auto it = m_Viewports.begin(); it != m_Viewports.end(); ++it)
    {
        if ((*it)->GetName() == Name)
        {
            return *it;
        }
    }

    return nullptr;
}

}
