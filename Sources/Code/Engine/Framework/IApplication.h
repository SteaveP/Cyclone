#pragma once

namespace Cyclone
{

class IPlatform;
class IRenderer;
class IWindow;
class IInputManager;
class IInputHandler;
class IUIModule;

class IApplication
{
public:
    virtual ~IApplication() = default;

    virtual int Run() = 0;

    virtual IPlatform* GetPlatform() = 0;
    virtual IWindow* GetWindow() = 0;
    virtual IRenderer* GetRenderer() = 0;

    virtual IUIModule* GetUI() = 0;

    virtual double GetDeltaTime() const = 0;

    virtual IInputHandler* GetInputHandler() = 0;
    virtual IInputManager* GetInputManager() = 0;
};

} // namespace Cyclone
