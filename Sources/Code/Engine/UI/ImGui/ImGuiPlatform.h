#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Helpers.h"

namespace Cyclone
{

class IWindow;
class IPlatform;
class IUIModule;

class ENGINE_API ImGUIPlatform
{
public:
    ~ImGUIPlatform() = default;

    virtual C_STATUS OnInit(void* Instance, IUIModule* UIModule, IPlatform* Platform, IWindow* window) = 0;
    virtual C_STATUS OnFrame(void* Instance) = 0;
    virtual C_STATUS OnRender(void* Instance) = 0;
    virtual C_STATUS OnWindowMessage(void* Instance, void* DataPtr) = 0;
    virtual C_STATUS OnShutdown(void* Instance, IWindow* window) = 0;
};

} // namespace Cyclone
