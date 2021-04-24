#pragma once

#include "Engine/EngineModule.h"
#include "IWindow.h"

namespace Cyclone
{

class ENGINE_API IPlatform
{
public:
    virtual ~IPlatform() = default;

    virtual WindowPtr CreateWindowPtr() = 0;

    virtual void ChangeWorkingDirectory(std::string_view path) = 0;
};

ENGINE_API IPlatform* GEngineGetCurrentPlatform();
ENGINE_API void GEngineSetCurrentPlatform(IPlatform* Platform);


} // namespace Cyclone
