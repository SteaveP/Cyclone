#include "IUIModule.h"

#include "Engine/Core/Helpers.h"

namespace Cyclone
{

static IUIModule* GCurrentUIModule = nullptr;

ENGINE_API IUIModule* GEngineGetCurrentUIModule()
{
    return GCurrentUIModule;
}

ENGINE_API void GEngineSetCurrentUIModule(IUIModule* RenderBackend)
{
    CASSERT(GCurrentUIModule == nullptr);
    GCurrentUIModule = RenderBackend;
}

} // namespace Cyclone
