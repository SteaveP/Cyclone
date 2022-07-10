#pragma once

#include "PlatformWinModuleDefines.h"
#include "Engine/Framework/IPlatform.h"

#include "Common/CommonWin.h"

namespace Cyclone
{

class PlatformWin : public IPlatform
{
public:
    DISABLE_COPY_ENABLE_MOVE(PlatformWin);

    PlatformWin();

    virtual UniquePtr<IWindow> CreateWindowPtr() override;
    virtual Ptr<IEvent> CreateEventPtr() override;

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
