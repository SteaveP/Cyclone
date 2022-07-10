#pragma once

//#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// Engine
#include "Engine/Engine.h"
#include "Engine/Core/Types.h"

// This module
#include "Engine/Platform/Impl/Windows/PlatformWinModuleDefines.h"

struct CPlatformDataWin
{
    HINSTANCE hInstance;
    int nCmdShow;
};
