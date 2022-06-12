#pragma once

#include "PlatformWinModule.h"
#include "Engine/Framework/IPlatform.h"

namespace Cyclone
{

class PLATFORMWIN_API PlatformWin : public IPlatform
{
public:
    virtual WindowPtr CreateWindowPtr() override;

    virtual void ChangeWorkingDirectory(std::string_view Path) override;

    virtual void SetOnWindowMessageCallback(OnWindowMessageCallback Callback) override;
    virtual C_STATUS OnWindowMessage(IWindow* Window, void* DataPtr) override;

    virtual void SetOnDPIChangedCallback(OnDPIChangedCallback Callback) override;
    virtual C_STATUS OnDPIChanged(float NewDPI, float OldDPI) override;

protected:
    OnWindowMessageCallback m_OnWindowMessageCallback;
    OnDPIChangedCallback m_OnDPIChangedCallback;
};

} // namespace Cyclone
