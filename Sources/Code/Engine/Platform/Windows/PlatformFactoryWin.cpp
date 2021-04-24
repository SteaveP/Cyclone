#include "PlatformFactoryWin.h"

#include "Engine/EngineModule.h"
#include "CommonWin.h"
#include "WindowWin.h"

namespace Cyclone
{

// global instance
static PlatformFactoryWin GPlatformFactoryWin;

Cyclone::WindowPtr PlatformFactoryWin::CreateWindowPtr()
{
    return std::make_unique<WindowWinApi>();
}

void GInitPlatformFactoryWin()
{
    GEngineSetPlatformFactory(&GPlatformFactoryWin);
}

} // namespace Cyclone
