#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Helpers.h"

namespace Cyclone
{

class IApplication;

class ENGINE_API IUISubsystem
{
public:
    DISABLE_COPY_ENABLE_MOVE(IUISubsystem);

    IUISubsystem() = default;
    virtual ~IUISubsystem() = default;

    virtual C_STATUS Init(IApplication* App, float Dpi = 96.f) = 0;
    virtual void Shutdown() noexcept = 0;

    virtual C_STATUS OnFrame() = 0;
    virtual C_STATUS OnRender() = 0;

    virtual C_STATUS OnWindowMessage(void* Params) = 0;
    virtual C_STATUS OnDPIChanged(float NewDPI, float OldDPI) = 0;
};

ENGINE_API IUISubsystem* GEngineGetCurrentUISubsystem();
ENGINE_API void GEngineSetCurrentUISubsystem(IUISubsystem* UIModule);

} // namespace Cyclone
