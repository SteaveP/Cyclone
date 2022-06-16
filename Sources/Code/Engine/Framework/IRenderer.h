#pragma once

#include "Engine/Core/ErrorCodes.h"
#include "Engine/Core/Types.h"
#include "Engine/EngineModule.h"

namespace Cyclone
{

namespace Render
{

class IRendererBackend;
class CRenderScene;
class CRenderSceneView;
class CSceneRenderer;
class CWindowContext;
class CCommandQueue;
enum class CommandQueueType;

enum class RasterizerState;
enum class BlendState;
enum class DepthStencilState;
class CRasterizerState;
class CBlendState;
class CDepthStencilState;

} // namespace Render

class IWindow;
class IApplication;
class CSceneViewport;
class CScene;

struct RendererDesc
{
    Vector<IWindow*> Windows;
    IApplication* App;
    unsigned int FrameCount;

    RendererDesc& SetWindows(Vector<IWindow*> W) { Windows = MoveTemp(W); return *this; }
    RendererDesc& SetApplication(IApplication* A) { App = A; return *this; }
    RendererDesc& SetFrameCount(int FrameCount) { FrameCount = FrameCount; return *this; }
};

class ENGINE_API IRenderer
{
public:
    DISABLE_COPY_ENABLE_MOVE(IRenderer);

    IRenderer() = default;
    virtual ~IRenderer() = default;

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

    virtual Render::CWindowContext* OnAddWindow(IWindow* Window) = 0;
    virtual void OnRemoveWindow(IWindow* Window) = 0;

    virtual Render::CSceneRenderer* GetSceneRenderer() = 0;

    virtual Render::CWindowContext* GetWindowContext(IWindow* Window) = 0;
    virtual Render::CWindowContext* GetDefaultWindowContext() = 0;
    virtual Render::CCommandQueue* GetDefaultCommandQueue(Render::CommandQueueType Type) = 0;

    virtual Render::CRasterizerState* GetRasterizerState(Render::RasterizerState State) = 0;
    virtual Render::CBlendState* GetBlendState(Render::BlendState State) = 0;
    virtual Render::CDepthStencilState* GetDepthStencilState(Render::DepthStencilState State) = 0;
};

} // namespace Cyclone
