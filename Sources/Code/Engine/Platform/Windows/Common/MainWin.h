#pragma once

// this file defines entry point for windows platform

#include "Engine/Startup/PlatformIndependentMain.h"
#include "CommonWin.h"

struct PlatformData
{
    HINSTANCE hInstance;
    int nCmdShow;
};

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);

    int argc = 0;

    // #todo_win
    //char* argv = CommandLineToArgvW(lpCmdLine, &argc);
    char* argv[] = { 0 };

    PlatformData Data
    {
        hInstance,
        nCmdShow
    };

    // MainCallback should be included before including this file in the caller
    int result = Cyclone::PlatformIndependentMain(argc, argv, &Data, MainCallback);

    // #todo_win
    //LocalFree(argv);
    
    return result;
}
