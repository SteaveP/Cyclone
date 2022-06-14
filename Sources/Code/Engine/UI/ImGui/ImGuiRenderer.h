#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Helpers.h"

namespace Cyclone
{

class IWindow;
class IUISubsystem;
namespace Render { class IRendererBackend; }

class ENGINE_API ImGUIRenderer
{
public:
    ~ImGUIRenderer() = default;

    virtual C_STATUS OnInit(void* Instance, IUISubsystem* UISubsystem,  Render::IRendererBackend* Backend, IWindow* Window) = 0;
    virtual C_STATUS OnFrame(void* Instance) = 0;
    virtual C_STATUS OnRender(void* Instance) = 0;
    virtual C_STATUS OnShutdown(void* Instance, IWindow* Window) = 0;
};

} // namespace Cyclone
