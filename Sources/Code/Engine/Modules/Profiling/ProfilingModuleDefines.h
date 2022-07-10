#pragma once

#include "Engine/Core/Helpers.h"

#ifdef DYNAMIC_LIB
#ifdef DYNAMIC_LIB_PROFILING
#define PROFILING_API DLL_EXPORT
#else
#define PROFILING_API DLL_IMPORT
#endif
#else
#define PROFILING_API
#endif
