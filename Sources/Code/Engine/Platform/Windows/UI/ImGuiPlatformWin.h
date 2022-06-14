#pragma once

#include "Engine/UI/ImGui/ImGuiPlatform.h"

namespace Cyclone
{

class ImGUIPlatformWin : public ImGUIPlatform
{
public:
    virtual C_STATUS OnInit(void* Instance, IUISubsystem* UIModule, IPlatform* Platform, IWindow* window) override;
    virtual C_STATUS OnFrame(void* Instance) override;
    virtual C_STATUS OnRender(void* Instance) override;
    virtual C_STATUS OnWindowMessage(void* Instance, void* DataPtr) override;
    virtual C_STATUS OnShutdown(void* Instance, IWindow* window) override;

protected:
    IUISubsystem* m_Module;
};

} // namespace Cyclone
