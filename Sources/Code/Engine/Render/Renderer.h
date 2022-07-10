#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Framework/IRenderer.h"
#include "Engine/Utils/Delegate.h"

#include "CommonRender.h"
#include "Utils/Pool.h"

#include <thread>

namespace Cyclone::Render
{

class ENGINE_API CRenderer : public IRenderer
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CRenderer);

    CRenderer();
    virtual ~CRenderer();

    //////////////////////////////////////////////////////////////////////////
    // Can be called from any thread group
    //////////////////////////////////////////////////////////////////////////

    virtual C_STATUS PreInit(IRendererBackend* RendererBackend);

    // IRenderer
    virtual C_STATUS Init(const CRendererDesc* desc) override;
    virtual void DeInit() override;

    virtual C_STATUS EnqueueFrame() override;
    virtual C_STATUS EnqueueFrameUI() override;
    virtual C_STATUS WaitFrame(bool ShouldTerminate = false) override;

    virtual void OnWindowAdded(IWindow* Window) override;
    virtual void OnWindowRemoved(IWindow* Window) override;

    virtual C_STATUS OnWindowResize(IWindow* Window) override;

    virtual void WaitGPU() override;

    virtual IApplication* GetApp() const override { return m_App; }
    virtual IRendererBackend* GetRendererBackend() override { return m_RendererBackend; }

    virtual uint32 GetCurrentFrame() const override { return m_CurrentFrame; }
    virtual uint32 GetLastCompletedFrame() const override { return m_CurrentFrame - (m_CurrentFrame <= GetFramesInFlightCount()
        ? 0 : GetFramesInFlightCount()); } // #todo_vk implement a fence for this
    virtual uint32 GetFramesInFlightCount() const override { return m_FramesInFlightCount; }
    virtual uint32 GetCurrentLocalFrame() const override { return m_CurrentLocalFrame; }

    virtual CRenderDelegate* GetOnBeginRenderDelegate() override { return &m_OnBeginRenderDelegate; }
    virtual CRenderDelegate* GetOnRenderDelegate() override { return &m_OnRenderDelegate;}
    virtual CRenderDelegate* GetOnEndRenderDelegate() override { return &m_OnEndRenderDelegate;}

    virtual CWindowContextDelegate* GetOnWindowContextAddedDelegate() override { return &m_OnWindowContextAddedDelegate; }
    virtual CWindowContextDelegate* GetOnWindowContextRemovedDelegate() override { return &m_OnWindowContextRemovedDelegate; }

    //////////////////////////////////////////////////////////////////////////
    // Should be called from RenderThread group
    //////////////////////////////////////////////////////////////////////////

    virtual C_STATUS RenderFrame() override;

    virtual CWindowContext* GetWindowContext(IWindow* Window) override;
    virtual CWindowContext* GetDefaultWindowContext() override;

    virtual CCommandQueue* GetDefaultCommandQueue(CommandQueueType Type) override;

    virtual Render::CHandle<Render::CRasterizerState> CreateRasterizerState(Render::CRasterizerState State) override;
    virtual Render::CHandle<Render::CBlendState> CreateBlendState(Render::CBlendState State) override;
    virtual Render::CHandle<Render::CDepthStencilState> CreateDepthStencilState(Render::CDepthStencilState State) override;

    virtual void DestroyRasterizerState(Render::CHandle<Render::CRasterizerState> Handle) override;
    virtual void DestroyBlendState(Render::CHandle<Render::CBlendState> Handle) override;
    virtual void DestroyDepthStencilState(Render::CHandle<Render::CDepthStencilState> Handle) override;

    virtual const Render::CRasterizerState* GetRasterizerState(Render::CHandle<Render::CRasterizerState> Handle) const override;
    virtual const Render::CBlendState* GetBlendState(Render::CHandle<Render::CBlendState> Handle) const override;
    virtual const Render::CDepthStencilState* GetDepthStencilState(Render::CHandle<Render::CDepthStencilState> Handle) const override;

protected:
    virtual C_STATUS BeginRender();
    virtual C_STATUS Render();
    virtual C_STATUS EndRender();

private:
    void DeInitImpl() noexcept;

protected:
    IRendererBackend* m_RendererBackend = nullptr;
    IApplication* m_App = nullptr;

    Vector<CHandle<CWindowContext>> m_WindowContexts; // #todo_memory inline allocator

    bool m_EnableMultithreading = false;

    uint32 m_FramesInFlightCount = 0;
    uint32 m_CurrentLocalFrame = 0;   // Local frame counter [0, m_FramesInFlightCount)
    uint32 m_CurrentFrame = 0;        // Global frame counter

    UniquePtr<std::thread> m_RenderThread;

    CRenderDelegate m_OnBeginRenderDelegate;
    CRenderDelegate m_OnRenderDelegate;
    CRenderDelegate m_OnEndRenderDelegate;

    CWindowContextDelegate m_OnWindowContextAddedDelegate;
    CWindowContextDelegate m_OnWindowContextRemovedDelegate;

    //////////////////////////////////////////////////////////////////////////
    CPool<CRasterizerState,   CRasterizerState>    m_RasterizerStatePool;
    CPool<CBlendState,        CBlendState>         m_BlendStatePool;
    CPool<CDepthStencilState, CDepthStencilState>  m_DepthStencilStatePool;
};

} // namespace Cyclone::Render
