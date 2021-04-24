#include "PlatformWin.h"

#include "Engine/EngineModule.h"
#include "Common/CommonWin.h"
#include "Window/WindowWin.h"

#include <direct.h>

namespace Cyclone
{

// global instance
static PlatformWin GPlatformFactoryWin;

Cyclone::WindowPtr PlatformWin::CreateWindowPtr()
{
    return std::make_unique<WindowWinApi>();
}

void PlatformWin::ChangeWorkingDirectory(std::string_view path)
{
    chdir(path.data());
}

void GInitPlatformFactoryWin()
{
    GEngineSetCurrentPlatform(&GPlatformFactoryWin);
}

} // namespace Cyclone
