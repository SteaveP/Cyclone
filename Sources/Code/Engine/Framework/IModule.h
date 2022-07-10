#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

namespace Cyclone
{

class ENGINE_API IModule
{
public:
    DISABLE_COPY_ENABLE_MOVE(IModule);

    IModule() = delete;
	virtual ~IModule() = default;
    
    IModule(String Name) : m_Name(Name) {}

    virtual C_STATUS OnRegister() = 0;
    virtual C_STATUS OnUnRegister() = 0;

    virtual void OnBeginFrame() {};
    virtual void OnUpdate() {};
    virtual void OnRender() {};
    virtual void OnEndFrame() {};

    String GetName() const { return m_Name; }

protected:
    String m_Name;
};

ENGINE_API void GEngineRegisterModule(IModule* Module);
ENGINE_API void GEngineUnRegisterModule(IModule* Module);
ENGINE_API void GEngineUnRegisterAllModules();

// don't use Vectors as export functions
ENGINE_API void GEngineGetModules(IModule**& Modules, uint32_t& ModulesCount);

inline ENGINE_API IModule* GEngineGetModule(std::string_view Name)
{
    IModule** Modules;
    uint32_t ModulesCount;
    GEngineGetModules(Modules, ModulesCount);

    for (uint32_t i = 0; i < ModulesCount; ++i)
    {
        if (Modules[i]->GetName() == Name)
            return Modules[i];
    }

    return nullptr;
}
	
} // namespace Cyclone
