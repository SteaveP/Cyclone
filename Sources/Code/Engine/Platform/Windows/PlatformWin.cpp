#include "PlatformWin.h"

#include "Common/CommonWin.h"
#include "Window/WindowWin.h"

#include <direct.h>

namespace Cyclone
{

Cyclone::WindowPtr PlatformWin::CreateWindowPtr()
{
    return std::make_unique<WindowWinApi>();
}

void PlatformWin::ChangeWorkingDirectory(std::string_view path)
{
    int result = chdir(path.data());
    CASSERT(result == 0);
}

void PlatformWin::SetOnWindowMessageCallback(OnWindowMessageCallback Callback)
{
    CASSERT(!m_onWindowMessageCallback);
    m_onWindowMessageCallback = Callback;
}

C_STATUS PlatformWin::OnWindowMessage(IWindow* Window, void* DataPtr)
{
    if (m_onWindowMessageCallback)
        m_onWindowMessageCallback(Window, DataPtr);

    return C_STATUS::C_STATUS_INVALID_ARG;
}

void PlatformWin::SetOnDPIChangedCallback(OnDPIChangedCallback Callback)
{
    CASSERT(!m_onDPIChangedCallback);
    m_onDPIChangedCallback = Callback;
}

C_STATUS PlatformWin::OnDPIChanged(float newDPI, float oldDPI)
{
    if (m_onDPIChangedCallback)
        m_onDPIChangedCallback(newDPI, oldDPI);

    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
