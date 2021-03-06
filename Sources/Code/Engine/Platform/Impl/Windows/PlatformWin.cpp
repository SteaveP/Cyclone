#include "PlatformWin.h"

#include "Window/WindowWin.h"
#include "EventWin.h"

#include <direct.h>

namespace Cyclone
{

PlatformWin::PlatformWin() = default;

UniquePtr<IWindow> PlatformWin::CreateWindowPtr()
{
    return MakeUnique<WindowWinApi>();
}

Ptr<Cyclone::IEvent> PlatformWin::CreateEventPtr()
{
    return MakeShared<CEventWin>();
}

void PlatformWin::ChangeWorkingDirectory(std::string_view Path)
{
    int result = chdir(Path.data());
    CASSERT(result == 0);
}

void PlatformWin::SetOnWindowMessageCallback(OnWindowMessageCallback Callback)
{
    CASSERT(!m_OnWindowMessageCallback);
    m_OnWindowMessageCallback = Callback;
}

C_STATUS PlatformWin::OnWindowMessage(IWindow* Window, void* DataPtr)
{
    if (m_OnWindowMessageCallback)
        m_OnWindowMessageCallback(Window, DataPtr);

    return C_STATUS::C_STATUS_INVALID_ARG;
}

void PlatformWin::SetOnDPIChangedCallback(OnDPIChangedCallback Callback)
{
    CASSERT(!m_OnDPIChangedCallback);
    m_OnDPIChangedCallback = Callback;
}

C_STATUS PlatformWin::OnDPIChanged(float NewDPI, float OldDPI)
{
    if (m_OnDPIChangedCallback)
        m_OnDPIChangedCallback(NewDPI, OldDPI);

    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
