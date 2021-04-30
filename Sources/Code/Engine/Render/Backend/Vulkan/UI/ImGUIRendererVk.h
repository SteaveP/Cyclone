#pragma once

#include "../RenderBackendVkModule.h"
#include "Engine/UI/ImGui/ImGuiRenderer.h"

namespace Cyclone::Render
{

class RENDER_BACKEND_VK_API ImGUIRendererVk : public ImGUIRenderer
{
public:
	ImGUIRendererVk();
	~ImGUIRendererVk();

	C_STATUS OnInit(void* Instance, IUIModule* UIModule, Render::IRendererBackend* Backend, IWindow* window) override;
	C_STATUS OnFrame(void* Instance) override;
	C_STATUS OnRender(void* Instance) override;
	C_STATUS OnShutdown(void* Instance, IWindow* window) override;

private:
	// can't use std::unique_ptr due to DLL-export issues
	class Pimpl;
	Pimpl* m_pimpl = nullptr;
	IUIModule* m_module;
};

} // namespace Cyclone::Render
