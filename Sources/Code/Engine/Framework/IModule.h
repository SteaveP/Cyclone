#pragma once

#include "Engine/EngineModule.h"

namespace Cyclone
{

class ENGINE_API IModule
{
public:
	virtual ~IModule() = default;

    virtual C_STATUS OnRegister() = 0;
    virtual C_STATUS OnUnRegister() = 0;
};

ENGINE_API void GEngineRegisterModule(IModule* Module);
ENGINE_API void GEngineUnRegisterModule(IModule* Module);
	
} // namespace Cyclone
