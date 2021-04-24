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
    int result = chdir(path.data());
    CASSERT(result == 0);
}

Cyclone::C_STATUS PlatformWinModule::OnRegister()
{
#ifdef _DEBUG
    printf("Module: PlatformWinModule registered\n");
#endif

    CASSERT(GEngineGetCurrentPlatform() == nullptr);
    GEngineSetCurrentPlatform(&GPlatformFactoryWin);
    return C_STATUS::C_STATUS_OK;
}

Cyclone::C_STATUS PlatformWinModule::OnUnRegister()
{
#ifdef _DEBUG
    printf("Module: PlatformWinModule unregistered\n");
#endif

    CASSERT(GEngineGetCurrentPlatform() == &GPlatformFactoryWin);
    GEngineSetCurrentPlatform(nullptr);
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
