#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

#include <functional>

namespace Cyclone
{

class IEvent;
class IModule;
class IWindow;

class ENGINE_API IPlatform
{
public:
    using OnWindowMessageCallback = std::function<C_STATUS(IWindow* Window, void* DataPtr)>;
    using OnDPIChangedCallback    = std::function<C_STATUS(float newDPI, float oldDPI)>;

public:
    DISABLE_COPY_ENABLE_MOVE(IPlatform);

    IPlatform() = default;
    virtual ~IPlatform() = default;

    virtual UniquePtr<IWindow> CreateWindowPtr() = 0;
    virtual Ptr<IEvent> CreateEventPtr() = 0;

    virtual void ChangeWorkingDirectory(std::string_view path) = 0;

    virtual void SetOnWindowMessageCallback(OnWindowMessageCallback Callback) = 0;
    virtual C_STATUS OnWindowMessage(IWindow* Window, void* DataPtr) = 0;

    virtual void SetOnDPIChangedCallback(OnDPIChangedCallback Callback) = 0;
    virtual C_STATUS OnDPIChanged(float newDPI, float oldDPI) = 0;
};

ENGINE_API IPlatform* GEngineGetCurrentPlatform();
ENGINE_API void GEngineSetCurrentPlatform(IPlatform* Platform);

} // namespace Cyclone
