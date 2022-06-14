#include "Engine/Startup/PlatformIndependentMain.h"

// Include this file into the application project and define GENERATE_MAIN_FUNCTION
// to generate default platform main methods

#ifdef GENERATE_MAIN_FUNCTION

#ifdef GENERATE_DEFAULT_MODULE_LOADER
    #include "DefaultModulesLoader.cpp"
#endif

Cyclone::MainEntryCallback MainCallback = [](int Argc, char* Argv[], void* PlatformDataPtr,
    Cyclone::Ptr<Cyclone::DefaultApplication>& App, Cyclone::DefaultApplicationParams& AppParams)
{
    C_UNREFERENCED(Argc);
    C_UNREFERENCED(Argv);
    C_UNREFERENCED(PlatformDataPtr);

#ifdef GENERATE_DEFAULT_MODULE_LOADER
    LoadModuleMainCallback(Argc, Argv, PlatformDataPtr, App, AppParams);
#endif

#ifdef APPLICATION_CRATE_CALLBACK
    APPLICATION_CRATE_CALLBACK(Argc, Argv, PlatformDataPtr, App, AppParams);
#endif
};

#ifdef PLATFORM_WIN64
    #include "Engine/Platform/Windows/Common/MainWin.h"
#else
    #error unsupported platform
#endif // PLATFORMS

#endif // GENERATE_MAIN_FUNCTION
