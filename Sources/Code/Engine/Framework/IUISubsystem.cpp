#include "IUISubsystem.h"

#include "Engine/Core/Helpers.h"

namespace Cyclone
{

static IUISubsystem* GCurrentUISubsystem = nullptr;

ENGINE_API IUISubsystem* GEngineGetCurrentUISubsystem()
{
    return GCurrentUISubsystem;
}

ENGINE_API void GEngineSetCurrentUISubsystem(IUISubsystem* RenderBackend)
{
    // #todo callback OnUISubsystemChanged?
    CASSERT(RenderBackend == nullptr || GCurrentUISubsystem == nullptr);
    GCurrentUISubsystem = RenderBackend;
}

} // namespace Cyclone
