#pragma once

#include "Engine/Framework/IUISubsystem.h"

struct ImGuiContext;

namespace Cyclone
{

class ImGUIRenderer;
class ImGUIPlatform;

class ENGINE_API ImGUISubsystem : public IUISubsystem
{
public:
    ~ImGUISubsystem() = default;

    void SetRenderer(ImGUIRenderer* Renderer) { m_Renderer = Renderer; }
    void SetPlatform(ImGUIPlatform* Platform) { m_Platform = Platform; }

    virtual C_STATUS Init(IApplication* App, float Dpi = 96.f) override;
    virtual void Shutdown() noexcept override;

    virtual C_STATUS OnFrame() override;
    virtual C_STATUS OnRender(Render::CCommandBuffer* CommandBuffer) override;

    virtual C_STATUS OnWindowMessage(void* Params) override;
    virtual C_STATUS OnDPIChanged(float NewDPI, float OldDPI) override;

protected:
    IApplication* m_App = nullptr;
    ImGuiContext* m_Context = nullptr;
    ImGUIRenderer* m_Renderer = nullptr;
    ImGUIPlatform* m_Platform = nullptr;
};

} // namespace Cyclone
