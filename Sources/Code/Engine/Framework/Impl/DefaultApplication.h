#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Engine.h"

#include <vector>

namespace Cyclone
{

class DefaultInputManager;

struct ENGINE_API DefaultApplicationParams
{
    IPlatform* Platform = nullptr;
    IUIModule* UIModule = nullptr;
    uint32 WindowsCount = 1;
    std::shared_ptr<IRenderer> Renderer;
    std::shared_ptr<DefaultInputManager> InputManager;

    void* PlatformStartupDataPtr = nullptr; // #todo_fixme
    std::string WindowCaption;
};

class ENGINE_API DefaultApplication : public IApplication
{
public:
    DefaultApplication();
    ~DefaultApplication();

    virtual C_STATUS Init(const DefaultApplicationParams& desc);
    void DeInit();

    // IApplication
    virtual int Run() override;

    virtual IPlatform* GetPlatform() override { return m_Platform; }
    virtual IWindow* GetWindow(uint32 Index) override { return m_Windows[Index].get(); }
    virtual uint32 GetWindowsCount() const { return static_cast<uint32>(m_Windows.size()); }
    virtual IRenderer* GetRenderer() override { return m_Renderer.get(); }

    virtual IUIModule* GetUI() override { return m_UI; } // #todo refactor

    virtual IInputHandler* GetInputHandler() override;
    virtual IInputManager* GetInputManager() override;

    virtual double GetDeltaTime() const override;

    bool IsInit() const { return m_IsInit; }
    double GetDT() const { return m_Dt; }

    void WaitAllPendingJobs();
    
protected:
    C_STATUS Frame();

    C_STATUS BeginFrame();
    C_STATUS Update();
    C_STATUS Render();
    C_STATUS EndFrame();

    // Application Callbacks
    virtual C_STATUS OnInit();
    virtual C_STATUS OnUpdate();
    virtual C_STATUS OnUpdateUI();
    virtual C_STATUS OnRender();

protected:
    bool m_IsInit;
    double m_Dt;

    IPlatform* m_Platform;
    IUIModule* m_UI;

    std::vector<std::shared_ptr<IWindow>> m_Windows;
    std::shared_ptr<IRenderer> m_Renderer;
    std::shared_ptr<DefaultInputManager> m_InputManager;
};

} // namespace Cyclone
