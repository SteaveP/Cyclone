#pragma once

namespace Cyclone
{

class IWindow;
class IRenderer;
class IInputManager;
class IInputHandler;

class IApplication
{
public:
    virtual ~IApplication() = default;

    virtual int Run() = 0;

    virtual IWindow* GetWindow() = 0;
    virtual IRenderer* GetRenderer() = 0;

    virtual double GetDeltaTime() const = 0;

    virtual IInputHandler* GetInputHandler() = 0;
    virtual IInputManager* GetInputManager() = 0;

    virtual void OnDPIChanged(float newDPI, float oldDPI) {};
};

} // namespace Cyclone
