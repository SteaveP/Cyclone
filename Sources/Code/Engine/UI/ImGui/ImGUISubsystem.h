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
    DISABLE_COPY_ENABLE_MOVE_DECL(ImGUISubsystem);

    ImGUISubsystem();
    ~ImGUISubsystem();

    void SetRenderer(ImGUIRenderer* Renderer) { m_Renderer = Renderer; }
    void SetPlatform(ImGUIPlatform* Platform) { m_Platform = Platform; }

    virtual C_STATUS Init(IApplication* App, float Dpi = 96.f) override;
    virtual void Shutdown() noexcept override;

    virtual C_STATUS OnFrame() override;
    virtual C_STATUS OnRender(Render::CCommandBuffer* CommandBuffer) override;

    virtual C_STATUS OnEndFrame() override;

    virtual C_STATUS OnWindowMessage(void* Params) override;
    virtual C_STATUS OnDPIChanged(float NewDPI, float OldDPI) override;

    virtual RawPtr RegisterTexture(Render::CHandle<Render::CResourceView> View, Render::EImageLayoutType ExpectedLayout) override;
    virtual void UnRegisterTexture(Render::CHandle<Render::CResourceView> View, RawPtr Descriptor) override;

private:
    void ShutdownImpl() noexcept;

protected:
    IApplication* m_App = nullptr;
    ImGuiContext* m_Context = nullptr;
    ImGUIRenderer* m_Renderer = nullptr;
    ImGUIPlatform* m_Platform = nullptr;
};

} // namespace Cyclone
