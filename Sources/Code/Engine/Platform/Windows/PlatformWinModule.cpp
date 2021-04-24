#include "PlatformWinModule.h"
#include "Engine/Platform/Windows/PlatformWin.h"

namespace Cyclone
{

// global instance
static PlatformWin GPlatformFactoryWin;

C_STATUS PlatformWinModule::OnRegister()
{
#ifdef _DEBUG
    printf("Module: PlatformWinModule registered\n");
#endif

    CASSERT(GEngineGetCurrentPlatform() == nullptr);
    GEngineSetCurrentPlatform(&GPlatformFactoryWin);
    return C_STATUS::C_STATUS_OK;
}

C_STATUS PlatformWinModule::OnUnRegister()
{
#ifdef _DEBUG
    printf("Module: PlatformWinModule unregistered\n");
#endif

    CASSERT(GEngineGetCurrentPlatform() == &GPlatformFactoryWin);
    GEngineSetCurrentPlatform(nullptr);
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
