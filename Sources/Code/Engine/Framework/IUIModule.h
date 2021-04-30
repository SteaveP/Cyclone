#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Helpers.h"

namespace Cyclone
{

class IApplication;

class ENGINE_API IUIModule
{
public:
    virtual ~IUIModule() = default;

    virtual C_STATUS Init(IApplication* app, float dpi = 96.f) = 0;
    virtual void Shutdown() noexcept = 0;

    virtual C_STATUS OnFrame() = 0;
    virtual C_STATUS OnRender() = 0;

    virtual C_STATUS OnWindowMessage(void* params) = 0;
    virtual C_STATUS OnDPIChanged(float newDPI, float oldDPI) = 0;
};

ENGINE_API IUIModule* GEngineGetCurrentUIModule();
ENGINE_API void GEngineSetCurrentUIModule(IUIModule* UIModule);

} // namespace Cyclone
