#include "Engine/Startup/PlatformIndependentMain.h"

// Include this file into the application project and define GENERATE_MAIN_FUNCTION
// to generate default platform main methods

// #todo custom startup callback or application type

#ifdef GENERATE_MAIN_FUNCTION
#ifdef PLATFORM_WIN64
#include "Engine/Platform/Windows/Common/MainWin.h"
#else
int main(int argc, char* argv[])
{
    return Cyclone::PlatformIndependentMain(argc, argv);
}
#endif // PLATFORMS
#endif // GENERATE_MAIN_FUNCTION
