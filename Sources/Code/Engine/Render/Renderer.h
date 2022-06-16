#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Framework/IRenderer.h"

#include "Common.h"

namespace Cyclone::Render
{

class ENGINE_API Renderer : public IRenderer
{
public:
    Renderer();
    virtual ~Renderer();

    virtual C_STATUS PreInit(IRendererBackend* RendererBackend);

    // IRenderer
    virtual C_STATUS Init(const RendererDesc* desc) override;

    virtual void Deinit() override;

    virtual C_STATUS BeginFrame() override;
    virtual C_STATUS EndFrame() override;

    virtual C_STATUS BeginRender() override;
    virtual C_STATUS Render() override;
    virtual C_STATUS EndRender() override;

    virtual uint32 GetCurrentFrame() const override { return m_CurrentFrame; }
    virtual uint32 GetCurrentLocalFrame() const override { return m_CurrentLocalFrame; }

    virtual void WaitGPU() override;
    
    virtual IRendererBackend* GetRendererBackend() override { return m_RendererBackend; }

    virtual IApplication* GetApp() const override { return m_App; }

    virtual CWindowContext* GetWindowContext(IWindow* Window) override;
    virtual CWindowContext* GetDefaultWindowContext() override;
    virtual CCommandQueue* GetDefaultCommandQueue(CommandQueueType Type) override;
    
    virtual CWindowContext* OnAddWindow(IWindow* Window) override;
    virtual void OnRemoveWindow(IWindow* Window) override;

    virtual CSceneRenderer* GetSceneRenderer() override;

    virtual CRasterizerState* GetRasterizerState(RasterizerState State) override;
    virtual CBlendState* GetBlendState(BlendState State) override;
    virtual CDepthStencilState* GetDepthStencilState(DepthStencilState State) override;
protected:
    IRendererBackend* m_RendererBackend = nullptr;
    IApplication* m_App = nullptr;

    Vector<IWindow*> m_Windows;
    Vector<UniquePtr<CWindowContext>> m_WindowContexts;

    UniquePtr<CSceneRenderer> m_SceneRenderer; // #todo_vk refactor

    uint32_t m_CurrentLocalFrame = 0;   // Local frame counter [0, MAX_FRAMES_IN_FLIGHT)
    uint32_t m_CurrentFrame = 0;        // Global frame counter
};

} // namespace Cyclone::Render
