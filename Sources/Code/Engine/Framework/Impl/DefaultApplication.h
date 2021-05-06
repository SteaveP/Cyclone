#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Engine.h"

namespace Cyclone
{

class DefaultInputManager;

struct ENGINE_API DefaultApplicationParams
{
    IPlatform* Platform;
    IUIModule* UIModule;
    std::shared_ptr<IRenderer> Renderer;
    std::shared_ptr<DefaultInputManager> InputManager;

    void* PlatformStartupDataPtr; // #todo_fixme
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

    virtual IPlatform* GetPlatform() override { return m_platform; }
    virtual IWindow* GetWindow() override { return m_window.get(); }
    virtual IRenderer* GetRenderer() override { return m_renderer.get(); }

    virtual IUIModule* GetUI() override { return m_ui; }

    virtual IInputHandler* GetInputHandler() override;
    virtual IInputManager* GetInputManager() override;

    virtual double GetDeltaTime() const override;

    bool IsInit() const { return m_isInit; }
    double GetDT() const { return m_dt; }

    void WaitAllPendingJobs();
    
protected:
    C_STATUS Frame();

    C_STATUS BeginFrame();
    C_STATUS Update();
    C_STATUS Render();
    C_STATUS EndFrame();

    // Callbacks
    virtual C_STATUS OnInit();
    virtual C_STATUS OnUpdate();
    virtual C_STATUS OnUpdateUI();
    virtual C_STATUS OnRender();

protected:
    bool m_isInit;
    double m_dt;

    IPlatform* m_platform;
    IUIModule* m_ui;

    std::shared_ptr<IWindow> m_window;
    std::shared_ptr<IRenderer> m_renderer;
    std::shared_ptr<DefaultInputManager> m_inputManager;
};

} // namespace Cyclone
