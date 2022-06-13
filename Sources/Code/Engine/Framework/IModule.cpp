#include "IModule.h"

#include "Engine/Core/Helpers.h"

#include <algorithm>

namespace Cyclone
{

static Vector<IModule*> GModules;

ENGINE_API void GEngineRegisterModule(IModule* Module)
{
    CASSERT(Module);
    if (Module)
    {
        GModules.push_back(Module);

        C_STATUS result = Module->OnRegister();
        CASSERT(C_SUCCEEDED(result));
    }
}
ENGINE_API void GEngineUnRegisterModule(IModule* Module)
{
    CASSERT(Module);
    if (Module)
    {
        C_STATUS result = Module->OnUnRegister();
        CASSERT(C_SUCCEEDED(result));

        GModules.erase(std::remove(GModules.begin(), GModules.end(), Module), GModules.end());
    }
}

ENGINE_API void GEngineGetModules(IModule**& Modules, uint32_t& ModulesCount)
{
    Modules = GModules.data();
    ModulesCount = (uint32_t)GModules.size();
}

} // namespace Cyclone
