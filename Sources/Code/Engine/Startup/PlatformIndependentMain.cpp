#include "PlatformIndependentMain.h"

#include "Engine/Framework/IUISubsystem.h"
#include "Engine/Framework/IPlatform.h"
#include "Engine/Framework/Impl/DefaultApplication.h"
#include "Engine/Framework/Impl/DefaultInputManager.h"

#include "Engine/Render/Backend/IRendererBackend.h"
#include "Engine/Render/Renderer.h"

#include "Engine/Utils/Profiling.h"
#include "Engine/Utils/Config.h"
#include "Engine/Utils/Log.h"

namespace Cyclone
{

#ifndef ENABLE_CONSOLE
#define ENABLE_CONSOLE 1
#endif

C_STATUS PreInitEnter(int32 ArgC, char* ArgV[])
{
    // Init CommandLine Arguments
    C_STATUS Result = GInitStartupArguments(ArgC, ArgV);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    // Change working directory
    // Executable runs from from <RootDir>/Bin to load binary dependencies (.dlls) from that folder
    // After that, need to change directory to <RootDir> to be able to reference assets and other project's files

    // Changing default working directory as early as possible
    // to be sure that loggers and configs will be loaded properly
    {
        String DefaultFolder = "..";
        GStartupArguments()->GetParameter("-DefaultFolder", DefaultFolder);

#if PLATFORM_WIN64 // #todo fixme
        int Res = chdir(DefaultFolder.c_str());
        CASSERT(Res == 0);

        #if ENABLE_CONSOLE
            AllocConsole();
            FILE* Dummy = nullptr;
            freopen_s(&Dummy, "CONOUT$", "w", stdout);
            freopen_s(&Dummy, "CONOUT$", "w", stderr);
            freopen_s(&Dummy, "CONIN$", "r", stdin);
            //std::cout.clear();
            //std::clog.clear();
            //std::cerr.clear();
            //std::cin.clear();
        #endif
#else
    #error Unsupported platform
#endif
    }

    // Init Config
    {
        String ConfigPath = "Config/Engine.json";
        GStartupArguments()->GetParameter("-Config", ConfigPath);
        Result = GInitConfig(ConfigPath);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    // Init logging
    String GreetingsString = "~~~Cyclone Engine~~~";
    {
        String LogPath = "Intermediate/Logs/Engine.log";
        LogPath = GET_CONFIG()["Log"].value("Filename", LogPath);
        GStartupArguments()->GetParameter("-LogFilename", LogPath);

        Result = Cyclone::GInitLogging(LogPath, GreetingsString);
        CASSERT(C_SUCCEEDED(Result));
    }

    // Put message about PreInit stage
    // We already in PreInit stage, but now we can put message in the log about this
    GLogger()->info(">>> Entering PreInit stage <<<");

    // Init profiling
    Result = GInitProfiling();
    CASSERT(C_SUCCEEDED(Result));

    return C_STATUS::C_STATUS_OK;
}

void PostDeInitEnter()
{
    spdlog::info(">>> Entering PostDeInit stage <<<");

    GDeInitProfiling();
    GDeInitLogging();
    GDeInitConfig();
    GDeInitStartupArguments();
}

// Note that working directory already should be changed to project's root at the moment of calling this function
int PlatformIndependentMain(int ArgC, char* ArgV[], void* PlatformDataRawPtr, MainEntryCallback EntryCallback)
{
    OPTICK_THREAD("MainThread");

    // DoPreInit stage
    C_STATUS Result = PreInitEnter(ArgC, ArgV);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), -1);

    UniquePtr<CDefaultApplication> App{};
    CDefaultApplicationParams Params{};

    if (EntryCallback)
    {
        EntryCallback(ArgC, ArgV, PlatformDataRawPtr, App, Params);
    }

    // Set default values if they wasn't been overridden
    if (Params.Platform == nullptr)
        Params.Platform = GEngineGetCurrentPlatform();

    if (Params.InputManager == nullptr)
        Params.InputManager = MakeShared<DefaultInputManager>();

    if (Params.UIModule == nullptr)
        Params.UIModule = GEngineGetCurrentUISubsystem();

    if (Params.Renderer == nullptr)
    {
        Ptr<Render::CRenderer> DefaultRenderer = MakeShared<Render::CRenderer>();
        DefaultRenderer->PreInit(GEngineGetCurrentRenderBackend());
        Params.Renderer = DefaultRenderer;
    }

    if (Params.ArgC < 0)
        Params.ArgC = ArgC;
    if (Params.ArgV == nullptr)
        Params.ArgV = ArgV;

    if (Params.PlatformStartupDataRawPtr == nullptr)
        Params.PlatformStartupDataRawPtr = PlatformDataRawPtr;

    if (App == nullptr)
        App = MakeUnique<CDefaultApplication>();

    // Do Init Stage
    spdlog::info(">>> Entering Init stage <<<");

    Result = App->Init(Params);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), -1);

    int ReturnValue = 0;

    if (Result == C_STATUS::C_STATUS_OK)
    {
        // Do Run stage
        spdlog::info(">>> Entering Run stage <<<");

        ReturnValue = App->Run();
        CASSERT(ReturnValue >= 0);
    }

    // Do DeInit stage
    {
        spdlog::info(">>> Entering DeInit stage <<<");

        App->DeInit();
    }

    // Do PostDeInit stage
    PostDeInitEnter();

    return ReturnValue;
}

} // namespace Cyclone
