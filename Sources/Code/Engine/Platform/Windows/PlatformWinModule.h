#pragma once

#include "PlatformWinModuleDefines.h"
#include "Engine/Framework/IModule.h"
#include "Engine/Core/Helpers.h"

namespace Cyclone
{

class IPlatform;
class ImGUIPlatform;

class PlatformWinModule : public IModule
{
public:
    PlatformWinModule() : IModule("PlatformWinModule") {}

    virtual C_STATUS OnRegister() override;
    virtual C_STATUS OnUnRegister() override;
};

PLATFORM_WIN_API IModule* CreatePlatformModule();
PLATFORM_WIN_API ImGUIPlatform* CreateImGUIPlatform();

} // namespace Cyclone
