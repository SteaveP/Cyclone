#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

namespace DelegateLib
{
    template<typename Param1>
    class MulticastDelegate1;
}

namespace Cyclone
{

class IPlatform;
class IRenderer;
class IWindow;
class ISubsystem;
class IInputManager;
class IInputHandler;
class IUISubsystem;
class CConfig;
class CCommandLineParams;

class ENGINE_API IApplication
{
public:
    DISABLE_COPY_ENABLE_MOVE(IApplication);

    IApplication() = default;
    virtual ~IApplication() = default;

    virtual int Run() = 0;

    virtual IPlatform* GetPlatform() = 0;
    virtual IRenderer* GetRenderer() = 0;
    virtual IUISubsystem* GetUI() = 0;

    virtual IWindow* GetWindow(uint32 Index) = 0;
    virtual uint32 GetWindowsCount() const = 0;

    virtual double GetDeltaTime() const = 0;

    virtual IInputHandler* GetInputHandler() = 0;
    virtual IInputManager* GetInputManager() = 0;

    virtual const CCommandLineParams* GetStartupArguments() const = 0;
    virtual const CConfig* GetConfig() const = 0;

    using CWindowDelegate = DelegateLib::MulticastDelegate1<IWindow*>;
    virtual CWindowDelegate* GetOnWindowAddedDelegate() = 0;
    virtual CWindowDelegate* GetOnWindowRemovedDelegate() = 0;

    // Subsystems
    virtual C_STATUS AddSubsystem(ISubsystem* Subsystem) = 0;
    virtual void RemoveSubsystem(ISubsystem* Subsystem) = 0;

    virtual ISubsystem* GetSubsystem(std::string_view Name) = 0;
    virtual ISubsystem* GetSubsystem(uint32 Index) = 0;
    virtual uint32 GetSubsystemCount() const = 0;

    using CSubsystemDelegate = DelegateLib::MulticastDelegate1<ISubsystem*>;
    virtual CSubsystemDelegate* GetOnSubsystemRegisteredDelegate() = 0;
    virtual CSubsystemDelegate* GetOnSubsystemUnRegisteredDelegate() = 0;
};

} // namespace Cyclone
