#pragma once

#include "Engine/EngineModule.h"
#include "IWindow.h"

namespace Cyclone
{

class ENGINE_API IPlatformFactory
{
public:
    virtual ~IPlatformFactory() = default;

    virtual WindowPtr CreateWindowPtr() = 0;
};

ENGINE_API IPlatformFactory* GEngineGetPlatformFactory();
ENGINE_API void GEngineSetPlatformFactory(IPlatformFactory* PlatformFactory);


} // namespace Cyclone
