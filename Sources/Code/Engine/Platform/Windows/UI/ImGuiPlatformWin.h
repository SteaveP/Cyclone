#pragma once

#include "../PlatformWinModule.h"
#include "Engine/UI/ImGui/ImGuiPlatform.h"

namespace Cyclone
{

class PLATFORMWIN_API ImGUIPlatformWin : public ImGUIPlatform
{
public:
    virtual C_STATUS OnInit(void* Instance, IUIModule* UIModule, IPlatform* Platform, IWindow* window) override;
    virtual C_STATUS OnFrame(void* Instance) override;
    virtual C_STATUS OnRender(void* Instance) override;
    virtual C_STATUS OnWindowMessage(void* Instance, void* DataPtr) override;
    virtual C_STATUS OnShutdown(void* Instance, IWindow* window) override;

protected:
    IUIModule* m_module;
};

} // namespace Cyclone
