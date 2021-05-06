#pragma once

#include "Engine/Core/ErrorCodes.h"

#include <string>
#include <memory>

namespace Cyclone
{

class IApplication;

struct WindowParams
{
    IApplication* app;
    int width;
    int height;

    std::string title;
};

typedef void* PlatformWindowHandle;

class IWindow
{
public:
    virtual ~IWindow() = default;

    virtual C_STATUS Init(const WindowParams* params) = 0;
    virtual void Deinit() = 0;

    virtual C_STATUS UpdateMessageQueue() = 0;

    virtual void OnUpdate() = 0;
    virtual void OnUpdateAfter() = 0;
    virtual void OnResize(unsigned int newWidth, unsigned int newHeight) = 0;
    virtual void OnDPIChanged(float newDPI, float oldDPI) = 0;

    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;

    virtual PlatformWindowHandle GetPlatformWindowHandle() const = 0;
    virtual IApplication* GetApp() const = 0;

    virtual void SetActive(bool active) = 0;
    virtual bool IsActive() const = 0;

    virtual void SetShowCursor(bool show) = 0;
    virtual bool GetShowCursor() const = 0;

    virtual void SetCenterCursor(bool centerCursor) = 0;
    virtual bool GetCenterCursor() const = 0;

    virtual float GetDPI() const = 0;
};

using WindowPtr = std::unique_ptr<IWindow>;

} // namespace Cyclone
