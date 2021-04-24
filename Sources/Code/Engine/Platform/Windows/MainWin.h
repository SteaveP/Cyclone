#pragma once

// this file defines entry point for windows platform

#include "Engine/Startup/PlatformIndependentMain.h"

#include "CommonWin.h"
#include "PlatformFactoryWin.h"

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);

    int argc = 0;

    Cyclone::GInitPlatformFactoryWin();

    // #todo_win
    //char* argv = CommandLineToArgvW(lpCmdLine, &argc);

    char* argv[] = { 0 };
    int result = Cyclone::PlatformIndependentMain(argc, argv);

    //LocalFree(argv);
    return result;
}
