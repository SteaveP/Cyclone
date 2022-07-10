#pragma once

#include "Engine/Core/ErrorCodes.h"
#include "Engine/Core/Types.h"
#include "Engine/EngineModule.h"
#include "Engine/Render/Handle.h"

namespace DelegateLib 
{ 
    class MulticastDelegate0;

    template<typename Param1>
    class MulticastDelegate1;
}

namespace Cyclone
{

namespace Render
{
    class IRendererBackend;
    class CWindowContext;
    class CCommandQueue;
    enum class CommandQueueType;

    enum class RasterizerState;
    enum class BlendState;
    enum class DepthStencilState;
    class CRasterizerState;
    class CBlendState;
    class CDepthStencilState;
    class CSceneRenderer;
} // namespace Render

class IWindow;
class IApplication;

struct CRendererDesc
{
    IApplication* App = nullptr;
    bool EnableMultiThreading = true;
    uint32 FramesInFlightCount = 2;

    CRendererDesc& SetApplication(IApplication* A) { App = A; return *this; }
    CRendererDesc& SetFramesInFlightCount(int FramesCount) { FramesInFlightCount = FramesInFlightCount; return *this; }
    CRendererDesc& SetMultiThreadingEnabled(bool Enabled) { EnableMultiThreading = Enabled; return *this; }
};

using CRenderDelegate = DelegateLib::MulticastDelegate0;
using CWindowContextDelegate = DelegateLib::MulticastDelegate1<Render::CHandle<Render::CWindowContext>>;

class ENGINE_API IRenderer
{
public:
    DISABLE_COPY_ENABLE_MOVE(IRenderer);

    //////////////////////////////////////////////////////////////////////////
    // Can be called from any thread group
    //////////////////////////////////////////////////////////////////////////
    IRenderer() = default;
    virtual ~IRenderer() = default;

    virtual C_STATUS Init(const CRendererDesc* desc) = 0;
    virtual void DeInit() = 0;

    virtual C_STATUS EnqueueFrame() = 0;
    virtual C_STATUS EnqueueFrameUI() = 0;
    virtual C_STATUS WaitFrame(bool ShouldTerminate = false) = 0;

    virtual void OnWindowAdded(IWindow* Window) = 0;
    virtual void OnWindowRemoved(IWindow* Window) = 0;

    virtual C_STATUS OnWindowResize(IWindow* Window) = 0;

    virtual void WaitGPU() = 0;

    virtual uint32 GetCurrentFrame() const = 0;
    virtual uint32 GetLastCompletedFrame() const = 0;
    virtual uint32 GetFramesInFlightCount() const = 0;
    virtual uint32 GetCurrentLocalFrame() const = 0;

    virtual IApplication* GetApp() const = 0;
    virtual Render::IRendererBackend* GetRendererBackend() = 0;

    virtual CRenderDelegate* GetOnBeginRenderDelegate() = 0;
    virtual CRenderDelegate* GetOnRenderDelegate() = 0;
    virtual CRenderDelegate* GetOnEndRenderDelegate() = 0;

    virtual CWindowContextDelegate* GetOnWindowContextAddedDelegate() = 0;
    virtual CWindowContextDelegate* GetOnWindowContextRemovedDelegate() = 0;

    //////////////////////////////////////////////////////////////////////////
    // Should be called from RenderThread group
    //////////////////////////////////////////////////////////////////////////
    virtual C_STATUS RenderFrame() = 0;

    virtual Render::CWindowContext* GetWindowContext(IWindow* Window) = 0;
    virtual Render::CWindowContext* GetDefaultWindowContext() = 0;

    virtual Render::CCommandQueue* GetDefaultCommandQueue(Render::CommandQueueType Type) = 0;

    // #todo_vk move to state manager?
    Render::CHandle<Render::CRasterizerState> RasterizerDefault;
    Render::CHandle<Render::CBlendState> BlendDisabled;
    Render::CHandle<Render::CDepthStencilState> DepthStencilDisabled;
    Render::CHandle<Render::CDepthStencilState> DepthStencilDepthRead;
    Render::CHandle<Render::CDepthStencilState> DepthStencilDepthWrite;

    virtual Render::CHandle<Render::CRasterizerState> CreateRasterizerState(Render::CRasterizerState State) = 0;
    virtual Render::CHandle<Render::CBlendState> CreateBlendState(Render::CBlendState State) = 0;
    virtual Render::CHandle<Render::CDepthStencilState> CreateDepthStencilState(Render::CDepthStencilState State) = 0;

    virtual void DestroyRasterizerState(Render::CHandle<Render::CRasterizerState> Handle) = 0;
    virtual void DestroyBlendState(Render::CHandle<Render::CBlendState> Handle) = 0;
    virtual void DestroyDepthStencilState(Render::CHandle<Render::CDepthStencilState> Handle) = 0;

    virtual const Render::CRasterizerState* GetRasterizerState(Render::CHandle<Render::CRasterizerState> Handle) const = 0;
    virtual const Render::CBlendState* GetBlendState(Render::CHandle<Render::CBlendState> Handle) const = 0;
    virtual const Render::CDepthStencilState* GetDepthStencilState(Render::CHandle<Render::CDepthStencilState> Handle) const = 0;
};

} // namespace Cyclone
