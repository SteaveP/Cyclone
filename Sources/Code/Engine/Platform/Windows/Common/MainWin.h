#pragma once

// this file defines entry point for windows platform

#include "Engine/Startup/PlatformIndependentMain.h"
#include "CommonWin.h"
#include <shellapi.h>
#include <cstdlib>

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

    static const int MAX_ARGUMENTS = 32;
    static const int MAX_ARGUMENT_LENGTH = 256;

    int Argc = 0;
    char* Argv[MAX_ARGUMENTS];

    char ArgvStorage[MAX_ARGUMENTS][MAX_ARGUMENT_LENGTH];
    {
        wchar_t** lpArgv = CommandLineToArgvW(lpCmdLine, &Argc);
        CASSERT(Argc <= MAX_ARGUMENTS);
        for (int i = 0; i < Argc; ++i)
        {
            size_t ConvertedCount = 0;
            wcstombs_s(&ConvertedCount, ArgvStorage[i], lpArgv[i], MAX_ARGUMENT_LENGTH);
            CASSERT(ConvertedCount <= MAX_ARGUMENT_LENGTH);
            Argv[i] = ArgvStorage[i];
        }

        LocalFree(lpArgv);
    }

    PlatformData Data { hInstance, nCmdShow };

    // MainCallback should be included before including this file in the caller
    int result = Cyclone::PlatformIndependentMain(Argc, Argv, &Data, MainCallback);
        
    return result;
}
