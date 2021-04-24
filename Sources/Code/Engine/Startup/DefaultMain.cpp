#include "Engine/Startup/PlatformIndependentMain.h"

// Include this file into the application project and define GENERATE_MAIN_FUNCTION
// to generate default platform main methods

#ifdef GENERATE_MAIN_FUNCTION

#ifdef GENERATE_DEFAULT_MODULE_LOADER
    #include "DefaultModulesLoader.cpp"
#else
    MainEntryCallback MainCallback = [](int argc, char* argv[], void* PlatformDataPtr)
    {
        C_UNREFERENCED(argc);
        C_UNREFERENCED(argv);
        C_UNREFERENCED(PlatformDataPtr);
    }
#endif // GENERATE_DEFAULT_MODULE_LOADER

#ifdef PLATFORM_WIN64
    #include "Engine/Platform/Windows/Common/MainWin.h"
#else
    #error unsupported platform
#endif // PLATFORMS

#endif // GENERATE_MAIN_FUNCTION
