#include "PlatformWinModule.h"
#include "PlatformWin.h"

#include "Engine/Utils/Log.h"
#include "UI/ImGuiPlatformWin.h"

namespace Cyclone
{

// Global instance
static PlatformWin GPlatformFactoryWin;

IModule* CreatePlatformModule()
{
    return new PlatformWinModule();
}

ImGUIPlatform* CreateImGUIPlatform()
{
    return new ImGUIPlatformWin();
}

C_STATUS PlatformWinModule::OnRegister()
{
    LOG_INFO("Module: PlatformWinModule registered");

    CASSERT(GEngineGetCurrentPlatform() == nullptr);
    GEngineSetCurrentPlatform(&GPlatformFactoryWin);
    return C_STATUS::C_STATUS_OK;
}

C_STATUS PlatformWinModule::OnUnRegister()
{
    LOG_INFO("Module: PlatformWinModule unregistered");

    CASSERT(GEngineGetCurrentPlatform() == &GPlatformFactoryWin);
    GEngineSetCurrentPlatform(nullptr);
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
