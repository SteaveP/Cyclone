#include "Engine/Startup/PlatformIndependentMain.h"

// Include this file into the application project and define GENERATE_MAIN_FUNCTION
// to generate default platform main methods

#ifdef GENERATE_MAIN_FUNCTION

#ifdef GENERATE_DEFAULT_MODULE_LOADER
    #include "DefaultModulesLoader.cpp"
#endif

Cyclone::MainEntryCallback MainCallback = [](int argc, char* argv[], void* PlatformDataPtr,
    std::shared_ptr<Cyclone::DefaultApplication>& App, Cyclone::DefaultApplicationParams& AppParams)
{
    C_UNREFERENCED(argc);
    C_UNREFERENCED(argv);
    C_UNREFERENCED(PlatformDataPtr);

#ifdef GENERATE_DEFAULT_MODULE_LOADER
    LoadModuleMainCallback(argc, argv, PlatformDataPtr, App, AppParams);
#endif

#ifdef APPLICATION_CRATE_CALLBACK
    APPLICATION_CRATE_CALLBACK(argc, argv, PlatformDataPtr, App, AppParams);
#endif
};

#ifdef PLATFORM_WIN64
    #include "Engine/Platform/Windows/Common/MainWin.h"
#else
    #error unsupported platform
#endif // PLATFORMS

#endif // GENERATE_MAIN_FUNCTION
