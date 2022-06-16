#pragma once

#include "ErrorCodes.h"
#include <assert.h>

namespace Cyclone
{

#define CASSERT(x) assert(x)

#define C_ASSERT_RETURN(x) if (!(x)) { CASSERT(x); return; }
#define C_ASSERT_RETURN_VAL(x, val) if (!(x)) { CASSERT(x); return (val); }

#define C_ASSERT_CONTINUE(x) if (!(x)) { CASSERT(x); continue; }
#define C_ASSERT_BREAK(x) if (!(x)) { CASSERT(x); break; }

#define C_SASSERT(x) static_assert(x)

// #todo use new c++ features instead of macros
#define KB *1024
#define MB KB KB

#define bit(x) (1 << (x))

#define C_MAX(x,y) ((x) >= (y) ? (x) : y)
#define C_MIN(x,y) ((x) <= (y) ? (x) : y)

#define DEFAULT_SYSTEM_ALIGNMENT 8

#define C_UNREFERENCED(x) (x)

#ifdef PLATFORM_WIN64
	#define DLL_IMPORT __declspec(dllimport) 
	#define DLL_EXPORT __declspec(dllexport)
#else
	#error unsupported platform
#endif

#define DISABLE_COPY(ClassName) \
    ClassName(const ClassName& Other) = delete; \
    ClassName& operator =(const ClassName& Other) = delete; \

#define DISABLE_MOVE(ClassName) \
    ClassName(ClassName&& Other) = delete; \
    ClassName& operator =(ClassName&& Other) = delete;

#define ENABLE_MOVE(ClassName) \
    ClassName(ClassName&& Other) = default; \
    ClassName& operator =(ClassName&& Other) = default;

#define DISABLE_COPY_ENABLE_MOVE(ClassName) \
    DISABLE_COPY(ClassName); \
    ENABLE_MOVE(ClassName);

#define DISABLE_COPY_DISABLE_MOVE(ClassName) \
    DISABLE_COPY(ClassName); \
    DISABLE_MOVE(ClassName);

#define ENABLE_DEBUG_RENDER_BACKEND 1

} // namespace Cyclone
