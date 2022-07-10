#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"
#include "Engine/Engine.h"

#include "Engine/Utils/Delegate.h"

namespace Cyclone
{

ENGINE_API CConfig* GConfig();
ENGINE_API CCommandLineParams* GStartupArguments();

class DefaultInputManager;

struct ENGINE_API CDefaultApplicationParams
{
    IPlatform* Platform = nullptr;
    IUISubsystem* UIModule = nullptr;
    uint32 WindowCount = 1;
    Ptr<IRenderer> Renderer;
    Ptr<DefaultInputManager> InputManager;

    String WindowCaption = "Cyclone's Window";
    void* PlatformStartupDataRawPtr = nullptr;

    bool OpenConsoleWindow = true;

    int ArgC = -1;
    char** ArgV = nullptr;
};

class ENGINE_API CDefaultApplication : public IApplication
{
public:
    CDefaultApplication();
    ~CDefaultApplication();

    virtual C_STATUS Init(const CDefaultApplicationParams& desc);
    virtual void DeInit();

    // IApplication
    virtual int Run() override;

    virtual IPlatform* GetPlatform() override { return m_Platform; }
    virtual IWindow* GetWindow(uint32 Index) override { return m_Windows[Index].get(); }
    virtual uint32 GetWindowsCount() const { return static_cast<uint32>(m_Windows.size()); }
    virtual IRenderer* GetRenderer() override { return m_Renderer.get(); }

    virtual IUISubsystem* GetUI() override { return m_UI; } // #todo_ui refactor

    virtual IInputHandler* GetInputHandler() override;
    virtual IInputManager* GetInputManager() override;

    virtual const CCommandLineParams* GetStartupArguments() const override { return GStartupArguments(); }
    virtual const CConfig* GetConfig() const { return GConfig(); }

    virtual double GetDeltaTime() const override;

    virtual CWindowDelegate* GetOnWindowAddedDelegate() override { return &m_OnWindowAddedDelegate; }
    virtual CWindowDelegate* GetOnWindowRemovedDelegate() override { return &m_OnWindowRemovedDelegate; }

    virtual C_STATUS AddSubsystem(ISubsystem* Subsystem) override;
    virtual void RemoveSubsystem(ISubsystem* Subsystem) override;

    virtual ISubsystem* GetSubsystem(std::string_view Name) override;
    virtual ISubsystem* GetSubsystem(uint32 Index) override { return m_Subsystems[Index].get(); }
    virtual uint32 GetSubsystemCount() const override { return (uint32)m_Subsystems.size(); }

    virtual CSubsystemDelegate* GetOnSubsystemRegisteredDelegate() override { return &m_OnSubsystemRegisteredDelegate; };
    virtual CSubsystemDelegate* GetOnSubsystemUnRegisteredDelegate() override { return &m_OnSubsystemUnRegisteredDelegate; };

    // Own methods
    bool IsInit() const { return m_IsInit; }
    double GetDT() const { return m_Dt; }

    void WaitAllPendingJobs();
    
protected:
    C_STATUS Frame();

    C_STATUS BeginFrame();
    C_STATUS Update();
    C_STATUS EndFrame();

    // Application Callbacks
    virtual C_STATUS OnAddSubsystems();
    virtual C_STATUS OnInit();
    virtual C_STATUS OnBeginFrame();
    virtual C_STATUS OnUpdate();
    virtual C_STATUS OnUpdateUI();
    virtual C_STATUS OnEndFrame();

private:
    void DeInitImpl();

protected:
    bool m_IsInit;
    double m_Dt;

    IPlatform* m_Platform = nullptr;
    IUISubsystem* m_UI = nullptr;

    Vector<Ptr<ISubsystem>> m_Subsystems;
    Vector<Ptr<IWindow>> m_Windows;
    Ptr<IRenderer> m_Renderer;
    Ptr<DefaultInputManager> m_InputManager;

    CSubsystemDelegate m_OnSubsystemRegisteredDelegate;
    CSubsystemDelegate m_OnSubsystemUnRegisteredDelegate;

    CWindowDelegate m_OnWindowAddedDelegate;
    CWindowDelegate m_OnWindowRemovedDelegate;
};

} // namespace Cyclone
