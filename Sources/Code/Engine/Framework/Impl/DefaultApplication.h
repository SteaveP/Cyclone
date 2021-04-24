#pragma once

#include "Engine/Framework/IApplication.h"
#include "Engine/Engine.h"

namespace Cyclone
{

class DefaultInputManager;

struct DefaultApplicationParams
{
    std::string windowCaption;
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

    std::unique_ptr<IWindow> m_window;
    std::unique_ptr<IRenderer> m_renderer;

    std::unique_ptr<DefaultInputManager> m_inputManager;
};

} // namespace Cyclone
