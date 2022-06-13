#pragma once

#include "Engine/Core/ErrorCodes.h"
#include "Engine/Core/Types.h"
#include "Engine/EngineModule.h"

namespace Cyclone
{

namespace Render
{
class IRendererBackend;
class IRendererBackendResource;
class WindowContext;
class CCommandQueue;
enum class CommandQueueType;
} // namespace Render

class IWindow;
class IApplication;
class ISceneRenderer;

struct RendererDesc
{
    Vector<IWindow*> Windows;
    IApplication* App;
    unsigned int FrameCount;

    RendererDesc& SetWindows(Vector<IWindow*> W) { Windows = std::move(W); return *this; }
    RendererDesc& SetApplication(IApplication* A) { App = A; return *this; }
    RendererDesc& SetFrameCount(int FrameCount) { FrameCount = FrameCount; return *this; }
};

class ENGINE_API IRenderer
{
public:
    IRenderer() = default;
    virtual ~IRenderer() = default;

    IRenderer(const IRenderer& other) = delete;
    IRenderer& operator = (const IRenderer& other) = delete;

    virtual C_STATUS Init(const RendererDesc* desc) = 0;
    virtual void Deinit() = 0;

    virtual IApplication* GetApp() const = 0;

    virtual C_STATUS BeginFrame() = 0;
    virtual C_STATUS EndFrame() = 0;

    virtual C_STATUS BeginRender() = 0;
    virtual C_STATUS Render() = 0;
    virtual C_STATUS EndRender() = 0;

    virtual uint32 GetCurrentFrame() const = 0;
    virtual uint32 GetCurrentLocalFrame() const = 0;

    virtual void WaitGPU() = 0;

    virtual Render::IRendererBackend* GetRendererBackend() = 0;

    virtual Render::WindowContext* GetWindowContext(IWindow* Window) = 0;
    virtual Render::WindowContext* GetDefaultWindowContext() = 0;
    virtual Render::CCommandQueue* GetDefaultCommandQueue(Render::CommandQueueType Type) = 0;
};

} // namespace Cyclone
