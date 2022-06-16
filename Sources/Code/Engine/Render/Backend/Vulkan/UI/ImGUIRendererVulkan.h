#pragma once

#include "Engine/UI/ImGui/ImGuiRenderer.h"
#include "Engine/Core/Types.h"

namespace Cyclone::Render
{

class ImGUIRendererVulkan : public ImGUIRenderer
{
public:
	ImGUIRendererVulkan();
	~ImGUIRendererVulkan();

	C_STATUS OnInit(void* Instance, IUISubsystem* UISubsystem, Render::IRendererBackend* Backend, IWindow* Window) override;
	C_STATUS OnFrame(void* Instance) override;
	C_STATUS OnRender(void* Instance, CCommandBuffer* CommandBuffer) override;
	C_STATUS OnShutdown(void* Instance, IWindow* Window) override;

private:
	class Pimpl;
	UniquePtr<Pimpl> m_pimpl;
	IUISubsystem* m_UISubsystem = nullptr;
	IWindow* m_Window = nullptr;
};

} // namespace Cyclone::Render
