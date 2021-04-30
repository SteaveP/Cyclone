#pragma once

#include "Engine/Core/ErrorCodes.h"
#include "Engine/EngineModule.h"

namespace Cyclone
{

namespace Render
{
    class IRendererBackend;
} // namespace Render

class IWindow;
class IApplication;
class ISceneRenderer;

struct RendererDesc
{
    IWindow* Window;
    IApplication* App;
    unsigned int FrameCount;

    RendererDesc& SetWindow(IWindow* w) { Window = w; return *this; }
    RendererDesc& SetApplication(IApplication* a) { App = a; return *this; }
    RendererDesc& SetFrameCount(int fc) { FrameCount = fc; return *this; }
};

class ENGINE_API IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual C_STATUS Init(const RendererDesc* desc) = 0;
    virtual void Deinit() = 0;

    virtual C_STATUS BeginFrame() = 0;

    virtual C_STATUS BeginRender() = 0;
    virtual C_STATUS Render() = 0;
    virtual C_STATUS EndRender() = 0;

    virtual C_STATUS EndFrame() = 0;

    virtual void OnResize(const IWindow* window) = 0;

    virtual void WaitGPU() = 0;

    virtual Render::IRendererBackend* GetRendererBackend() = 0;

    virtual void SetSceneRenderer(ISceneRenderer* sceneRenderer) = 0;

    virtual IApplication* GetApp() const = 0;
    virtual ISceneRenderer* GetSceneRenderer() const = 0;
};

} // namespace Cyclone
