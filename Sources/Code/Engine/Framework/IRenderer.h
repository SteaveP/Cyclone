#pragma once

namespace Cyclone
{

class IWindow;
class IApplication;
class ISceneRenderer;
class UIRenderer;

struct RendererDesc
{
    IWindow* window;
    IApplication* app;
    unsigned int frameCount;
};

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual C_STATUS Init(const RendererDesc* desc) = 0;
    virtual void Deinit() = 0;

    virtual C_STATUS BeginFrame() = 0;
    virtual C_STATUS Render() = 0;
    virtual void OnResize(const IWindow* window) = 0;

    virtual void WaitGPU() = 0;

    virtual void SetSceneRenderer(ISceneRenderer* sceneRenderer) = 0;

    virtual IApplication* GetApp() const = 0;
    virtual ISceneRenderer* GetSceneRenderer() const = 0;

    virtual UIRenderer* GetUIRenderer() const = 0;
};

} // namespace Cyclone
