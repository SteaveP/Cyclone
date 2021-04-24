#include "IModule.h"

#include "Engine/Core/Helpers.h"

namespace Cyclone
{

ENGINE_API void GEngineRegisterModule(IModule* Module)
{
    CASSERT(Module);
    if (Module)
    {
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
    }
}

} // namespace Cyclone
