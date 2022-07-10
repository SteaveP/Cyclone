#include "Engine/Startup/PlatformIndependentMain.h"

// Include this file into the application project and define GENERATE_MAIN_FUNCTION
// to generate default platform main methods

// Additionally, there is possibility to override:
//  * Module loading logic by overriding MODULE_LOADING_CALLBACK to point function with MainEntryCallback compatible signature
//  * Application creation logic by overriding APPLICATION_CREATE_CALLBACK to point function with MainEntryCallback compatible signature

#ifdef GENERATE_MAIN_FUNCTION

#ifndef MODULE_LOADING_CALLBACK
    #define GENERATE_DEFAULT_MODULE_LOADER 1
    #include "DefaultModulesLoader.cpp"
#endif

Cyclone::MainEntryCallback MainCallback = [](int Argc, char* Argv[], void* PlatformDataRawPtr,
    Cyclone::UniquePtr<Cyclone::CDefaultApplication>& App, Cyclone::CDefaultApplicationParams& AppParams)
{
    C_UNREFERENCED(Argc);
    C_UNREFERENCED(Argv);
    C_UNREFERENCED(PlatformDataRawPtr);

#ifdef MODULE_LOADING_CALLBACK
    MODULE_LOADING_CALLBACK(Argc, Argv, PlatformDataRawPtr, App, AppParams);
#endif

#ifdef APPLICATION_CREATE_CALLBACK
    APPLICATION_CREATE_CALLBACK(Argc, Argv, PlatformDataRawPtr, App, AppParams);
#endif
};

#define MAIN_CALLBACK MainCallback

#ifdef PLATFORM_WIN64
    #include "Engine/Platform/Impl/Windows/Common/MainWin.h"
#else
    #error unsupported platform
#endif // PLATFORMS

#endif // GENERATE_MAIN_FUNCTION
