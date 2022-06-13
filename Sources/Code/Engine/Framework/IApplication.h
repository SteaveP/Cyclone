#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

namespace Cyclone
{

class IPlatform;
class IRenderer;
class IWindow;
class IInputManager;
class IInputHandler;
class IUIModule;

class ENGINE_API IApplication
{
public:
    IApplication() = default;
    virtual ~IApplication() = default;

    virtual int Run() = 0;

    virtual IPlatform* GetPlatform() = 0;
    virtual IWindow* GetWindow(uint32 Index) = 0;
    virtual uint32 GetWindowsCount() const = 0;
    virtual IRenderer* GetRenderer() = 0;

    virtual IUIModule* GetUI() = 0;

    virtual double GetDeltaTime() const = 0;

    virtual IInputHandler* GetInputHandler() = 0;
    virtual IInputManager* GetInputManager() = 0;
};

} // namespace Cyclone
