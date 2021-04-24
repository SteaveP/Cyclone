#ifdef GENERATE_DEFAULT_MODULE_LOADER

#include "Engine/Core/Helpers.h"
#include "Engine/Framework/IModule.h"

// #todo temporal workaround for modules loading

#if PLATFORM_WIN64
#include "Engine/Platform/Windows/PlatformWin.h"
#endif

#if RENDER_BACKEND_VULKAN
#include "Engine/Render/Backend/Vulkan/RenderBackendVk.h"
#endif

Cyclone::MainEntryCallback MainCallback = [](int argc, char* argv[], void* PlatformDataPtr)
{
    C_UNREFERENCED(argc);
    C_UNREFERENCED(argv);
    C_UNREFERENCED(PlatformDataPtr);

#if PLATFORM_WIN64
    Cyclone::GEngineRegisterModule(new Cyclone::PlatformWinModule());
#endif

#if RENDER_BACKEND_VULKAN
    Cyclone::GEngineRegisterModule(new Cyclone::RenderBackendVulkanModule());
#endif
};

#endif // GENERATE_DEFAULT_MODULE_LOADER
