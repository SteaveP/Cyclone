#include "IPlatformFactory.h"

#include "Engine/Core/Helpers.h"

namespace Cyclone
{

IPlatformFactory* GPlatformFactory = nullptr;

IPlatformFactory* GEngineGetPlatformFactory()
{
    return GPlatformFactory;
}

void GEngineSetPlatformFactory(IPlatformFactory* PlatformFactory)
{
    CASSERT(GPlatformFactory == nullptr);
    GPlatformFactory = PlatformFactory;
}

} // namespace Cyclone
