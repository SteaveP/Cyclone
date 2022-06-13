#pragma once

#include "Engine/EngineModule.h"
#include "IWindow.h"

#include <functional>

namespace Cyclone
{

class ENGINE_API IPlatform
{
public:
    // #todo_ui fix warnings
    using OnWindowMessageCallback = std::function<C_STATUS(IWindow* Window, void* DataPtr)>; //C_STATUS(*)(IWindow*, void*);
    using OnDPIChangedCallback    = std::function<C_STATUS(float newDPI, float oldDPI)>; //C_STATUS(*)(float, float);

public:
    virtual ~IPlatform() = default;

    virtual UniquePtr<IWindow> CreateWindowPtr() = 0;

    virtual void ChangeWorkingDirectory(std::string_view path) = 0;

    virtual void SetOnWindowMessageCallback(OnWindowMessageCallback Callback) = 0;
    virtual C_STATUS OnWindowMessage(IWindow* Window, void* DataPtr) = 0;

    virtual void SetOnDPIChangedCallback(OnDPIChangedCallback Callback) = 0;
    virtual C_STATUS OnDPIChanged(float newDPI, float oldDPI) = 0;
};

ENGINE_API IPlatform* GEngineGetCurrentPlatform();
ENGINE_API void GEngineSetCurrentPlatform(IPlatform* Platform);


} // namespace Cyclone
