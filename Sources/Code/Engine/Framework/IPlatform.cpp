#include "IPlatform.h"

#include "Engine/Core/Helpers.h"

namespace Cyclone
{

static IPlatform* GCurrentPlatform = nullptr;

IPlatform* GEngineGetCurrentPlatform()
{
    return GCurrentPlatform;
}

void GEngineSetCurrentPlatform(IPlatform* PlatformFactory)
{
    CASSERT(GCurrentPlatform == nullptr);
    GCurrentPlatform = PlatformFactory;
}

} // namespace Cyclone
