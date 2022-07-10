#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"
#include "Engine/Core/Math.h"

namespace Cyclone
{

class IApplication;

struct WindowDesc
{
    IApplication* App = nullptr;
    Vec2i Size = { -1, -1 };

    void* PlatformDataPtr = nullptr;

    String Name;
    String Title;
};

typedef void* PlatformWindowHandle;

class ENGINE_API IWindow
{
public:
    DISABLE_COPY_ENABLE_MOVE(IWindow);

    IWindow() = default;
    virtual ~IWindow() = default;

    virtual C_STATUS Init(const WindowDesc* Params) = 0;
    virtual void DeInit() = 0;

    virtual C_STATUS UpdateMessageQueue() = 0;

    virtual void OnUpdate() = 0;
    virtual void OnUpdateAfter() = 0;
    virtual void OnResize(unsigned int NewWidth, unsigned int NewHeight) = 0;
    virtual void OnDPIChanged(float NewDPI, float OldDPI) = 0;

    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;

    virtual PlatformWindowHandle GetPlatformWindowHandle() const = 0;
    virtual IApplication* GetApp() const = 0;

    virtual const String& GetName() const = 0;

    virtual void SetActive(bool Active) = 0;
    virtual bool IsActive() const = 0;

    virtual void SetShowCursor(bool Show) = 0;
    virtual bool GetShowCursor() const = 0;

    virtual void SetCenterCursor(bool CenterCursor) = 0;
    virtual bool GetCenterCursor() const = 0;

    virtual float GetDPI() const = 0;
};

} // namespace Cyclone
