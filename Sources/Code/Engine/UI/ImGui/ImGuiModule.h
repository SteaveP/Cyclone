#pragma once

#include "Engine/Framework/IUIModule.h"

struct ImGuiContext;

namespace Cyclone
{

class ImGUIRenderer;
class ImGUIPlatform;

// #todo_ui should be IModule also?
class ENGINE_API ImGUIModule : public IUIModule
{
public:
    ~ImGUIModule() = default;

    void SetRenderer(ImGUIRenderer* Renderer) { m_renderer = Renderer; }
    void SetPlatform(ImGUIPlatform* Platform) { m_platform = Platform; }

    virtual C_STATUS Init(IApplication* app, float dpi = 96.f) override;
    virtual void Shutdown() noexcept override;

    virtual C_STATUS OnFrame() override;
    virtual C_STATUS OnRender() override;

    virtual C_STATUS OnWindowMessage(void* params) override;
    virtual C_STATUS OnDPIChanged(float newDPI, float oldDPI) override;

protected:
    IApplication* m_app = nullptr;
    ImGuiContext* m_context = nullptr;
    ImGUIRenderer* m_renderer = nullptr;
    ImGUIPlatform* m_platform = nullptr;
};

} // namespace Cyclone
