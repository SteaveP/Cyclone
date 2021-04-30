#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Framework/IRenderer.h"
#include "Engine/Render/IRendererBackend.h"

namespace Cyclone::Render
{

class ENGINE_API Renderer : public IRenderer
{
public:
    ~Renderer() = default;

    virtual C_STATUS PreInit(IRendererBackend* RendererBackend);

    // IRenderer
    virtual C_STATUS Init(const RendererDesc* desc) override;

    virtual void Deinit() override;

    virtual C_STATUS BeginFrame() override;
    virtual C_STATUS EndFrame() override;

    virtual C_STATUS BeginRender() override;
    virtual C_STATUS Render() override;
    virtual C_STATUS EndRender() override;

    virtual void OnResize(const IWindow* window) override;

    virtual void WaitGPU() override;
    
    virtual IRendererBackend* GetRendererBackend() override { return m_rendererBackend; }

    virtual IApplication* GetApp() const override { return m_app; }

    virtual void SetSceneRenderer(ISceneRenderer* sceneRenderer) override;
    virtual ISceneRenderer* GetSceneRenderer() const override;

protected:
    IRendererBackend* m_rendererBackend;
    IApplication* m_app;
    IWindow* m_window;
};

} // namespace Cyclone::Render
