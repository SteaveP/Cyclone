#pragma once

#include "Engine/Framework/IApplication.h"
#include "Engine/Engine.h"

namespace Cyclone
{

class DefaultInputManager;

struct DefaultApplicationParams
{
    IPlatform* Platform;
    std::shared_ptr<IRenderer> Renderer;
    std::shared_ptr<DefaultInputManager> InputManager;

    void* PlatformStartupDataPtr; // #todo_fixme
    std::string WindowCaption;
};

class DefaultApplication : public IApplication
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

    virtual IInputHandler* GetInputHandler() override;
    virtual IInputManager* GetInputManager() override;

    virtual double GetDeltaTime() const override;

    bool IsInit() const { return m_isInit; }
    double GetDT() const { return m_dt; }

    void WaitAllPendingJobs();
    
protected:
    C_STATUS Update();

    // IApplication
    virtual C_STATUS OnUpdate();
    virtual C_STATUS OnUpdateUI();
    virtual C_STATUS OnInit();

protected:
    bool m_isInit;
    double m_dt;

    IPlatform* m_platform;
    std::shared_ptr<IWindow> m_window;
    std::shared_ptr<IRenderer> m_renderer;

    std::shared_ptr<DefaultInputManager> m_inputManager;
};

} // namespace Cyclone
