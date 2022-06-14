#pragma once

#include "Engine/UI/ImGui/ImGuiRenderer.h"
#include "Engine/Core/Types.h"

namespace Cyclone::Render
{

class ImGUIRendererDX12 : public ImGUIRenderer
{
public:
	ImGUIRendererDX12();
	~ImGUIRendererDX12();

	C_STATUS OnInit(void* Instance, IUIModule* UIModule, Render::IRendererBackend* Backend, IWindow* Window) override;
	C_STATUS OnFrame(void* Instance) override;
	C_STATUS OnRender(void* Instance) override;
	C_STATUS OnShutdown(void* Instance, IWindow* Window) override;

private:
	class Pimpl;
	UniquePtr<Pimpl> m_pimpl;
	IUIModule* m_module = nullptr;
	IWindow* m_Window = nullptr;
};

} // namespace Cyclone::Render
