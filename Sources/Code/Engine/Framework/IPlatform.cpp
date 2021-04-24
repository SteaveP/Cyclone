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
    CASSERT(GCurrentPlatform == nullptr);
    GCurrentPlatform = Platform;
}

} // namespace Cyclone
