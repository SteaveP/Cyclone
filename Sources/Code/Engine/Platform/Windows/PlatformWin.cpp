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

} // namespace Cyclone
