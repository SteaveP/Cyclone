#pragma once

#include "Engine/UI/ImGui/ImGuiRenderer.h"
#include "Engine/Core/Types.h"

namespace Cyclone::Render
{

class CImGUIRendererVulkan : public ImGUIRenderer
{
public:
	CImGUIRendererVulkan();
	~CImGUIRendererVulkan();

	virtual C_STATUS OnInit(void* Instance, IUISubsystem* UISubsystem, Render::IRendererBackend* Backend, IWindow* Window) override;
	virtual C_STATUS OnShutdown(void* Instance, IWindow* Window) override;

	virtual C_STATUS OnFrame(void* Instance) override;
	virtual C_STATUS OnRender(void* Instance, CCommandBuffer* CommandBuffer) override;

    virtual RawPtr RegisterTexture(Render::CHandle<Render::CResourceView> View, EImageLayoutType ExpectedLayout) override;
    virtual void UnRegisterTexture(Render::CHandle<Render::CResourceView> View, RawPtr Descriptor) override;

private:
	class Pimpl;
	UniquePtr<Pimpl> m_pimpl;
	IUISubsystem* m_UISubsystem = nullptr;
	IWindow* m_Window = nullptr;
};

} // namespace Cyclone::Render
