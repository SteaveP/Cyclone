#include "IPlatform.h"

#include "Engine/Core/Helpers.h"

namespace Cyclone
{

static IPlatform* GCurrentPlatform = nullptr;

IPlatform* GEngineGetCurrentPlatform()
{
    return GCurrentPlatform;
}

void GEngineSetCurrentPlatform(IPlatform* Platform)
{
    // #todo callback OnPlatformChanged?
    CASSERT(Platform == nullptr || GCurrentPlatform == nullptr);
    GCurrentPlatform = Platform;
}

} // namespace Cyclone
