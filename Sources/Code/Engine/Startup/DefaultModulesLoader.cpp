#ifdef GENERATE_DEFAULT_MODULE_LOADER

#include "Engine/Core/Helpers.h"
#include "Engine/Framework/IModule.h"

// #todo temporal workaround for modules loading
#include "Engine/UI/ImGui/ImGuiModule.h"

#if PLATFORM_WIN64
#include "Engine/Platform/Windows/PlatformWinModule.h"
#include "Engine/Platform/Windows/UI/ImGuiPlatformWin.h"
#endif

#if RENDER_BACKEND_VULKAN
#include "Engine/Render/Backend/Vulkan/RenderBackendVkModule.h"
#include "Engine/Render/Backend/Vulkan/UI/ImGUIRendererVk.h"
#endif

Cyclone::MainEntryCallback LoadModuleMainCallback = [](int argc, char* argv[], void* PlatformDataPtr,
    std::shared_ptr<Cyclone::DefaultApplication>& App, Cyclone::DefaultApplicationParams& AppParams)
{
    C_UNREFERENCED(argc);
    C_UNREFERENCED(argv);
    C_UNREFERENCED(PlatformDataPtr);

    Cyclone::ImGUIModule* ImGUIModulePtr = new Cyclone::ImGUIModule();
    Cyclone::GEngineSetCurrentUIModule(ImGUIModulePtr);

#if PLATFORM_WIN64
    Cyclone::GEngineRegisterModule(new Cyclone::PlatformWinModule());
    ImGUIModulePtr->SetPlatform(new Cyclone::ImGUIPlatformWin());
#endif

#if RENDER_BACKEND_VULKAN
    Cyclone::GEngineRegisterModule(new Cyclone::RenderBackendVulkanModule());
    ImGUIModulePtr->SetRenderer(new Cyclone::Render::ImGUIRendererVk());
#endif
};

#endif // GENERATE_DEFAULT_MODULE_LOADER
