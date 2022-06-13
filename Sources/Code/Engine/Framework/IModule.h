#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

namespace Cyclone
{

class ENGINE_API IModule
{
public:
	virtual ~IModule() = default;

    virtual C_STATUS OnRegister() = 0;
    virtual C_STATUS OnUnRegister() = 0;

    virtual void OnBeginFrame() {};
    virtual void OnUpdate() {};
    virtual void OnRender() {};
    virtual void OnEndFrame() {};
};

ENGINE_API void GEngineRegisterModule(IModule* Module);
ENGINE_API void GEngineUnRegisterModule(IModule* Module);

// don't use Vectors as export functions
ENGINE_API void GEngineGetModules(IModule**& Modules, uint32_t& ModulesCount);

template<typename T>
inline ENGINE_API IModule* GEngineGetModule()
{
    IModule** Modules;
    uint32_t ModulesCount;
    GEngineGetModules(Modules, ModulesCount);

    for (uint32_t i = 0; i < ModulesCount; ++i)
    {
        if (dynamic_cast<std::remove_pointer<T>::type*>(Modules[i]))
            return Modules[i];
    }

    return nullptr;
}
	
} // namespace Cyclone
