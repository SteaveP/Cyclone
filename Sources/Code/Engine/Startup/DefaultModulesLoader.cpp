#ifdef GENERATE_DEFAULT_MODULE_LOADER

#include "Engine/Core/Helpers.h"
#include "Engine/Framework/IModule.h"

// #todo temporal workaround for modules loading

#define ADD_IMGUI_MODULE 1

#if ADD_IMGUI_MODULE
#include "Engine/UI/ImGui/ImGUIModule.h"
#endif

#if PLATFORM_WIN64
#include "Engine/Platform/Windows/PlatformWinModule.h"
#else
#error Unsupported platform
#endif

#if RENDER_BACKEND_VULKAN
#include "Engine/Render/Backend/Vulkan/RenderBackendVulkanModule.h"
#elif RENDER_BACKEND_DX12
#include "Engine/Render/Backend/DX12/RenderBackendDX12Module.h"
#else
#error Unsupported render backend
#endif

Cyclone::MainEntryCallback LoadModuleMainCallback = [](int Argc, char* Argv[], void* PlatformDataPtr,
    Cyclone::Ptr<Cyclone::DefaultApplication>& App, Cyclone::DefaultApplicationParams& AppParams)
{
    C_UNREFERENCED(Argc);
    C_UNREFERENCED(Argv);
    C_UNREFERENCED(PlatformDataPtr);

    Cyclone::GEngineRegisterModule(Cyclone::CreatePlatformModule());
    Cyclone::GEngineRegisterModule(Cyclone::CreateRenderBackendModule());

#if ADD_IMGUI_MODULE
    Cyclone::ImGUIModule* ImGUIModulePtr = new Cyclone::ImGUIModule();
    ImGUIModulePtr->SetPlatform(Cyclone::CreateImGUIPlatform());
    ImGUIModulePtr->SetRenderer(Cyclone::CreateImGUIRenderer());
    Cyclone::GEngineRegisterModule(ImGUIModulePtr);
#endif

};

#endif // GENERATE_DEFAULT_MODULE_LOADER
