#include "IModule.h"

#include "Engine/Core/Helpers.h"

#include <vector>
#include <algorithm>

namespace Cyclone
{

static std::vector<IModule*> g_Modules;

ENGINE_API void GEngineRegisterModule(IModule* Module)
{
    CASSERT(Module);
    if (Module)
    {
        g_Modules.push_back(Module);

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

        g_Modules.erase(std::remove(g_Modules.begin(), g_Modules.end(), Module), g_Modules.end());
    }
}

ENGINE_API void GEngineGetModules(IModule**& Modules, uint32_t& ModulesCount)
{
    Modules = g_Modules.data();
    ModulesCount = (uint32_t)g_Modules.size();
}

} // namespace Cyclone
