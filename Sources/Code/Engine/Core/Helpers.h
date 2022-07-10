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

// #todo use new c++ features instead of macroses
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
    ClassName(const ClassName& Other) /*noexcept*/ = delete; \
    ClassName& operator =(const ClassName& Other) /*noexcept*/ = delete; \

#define ENABLE_COPY(ClassName) \
    ClassName(const ClassName& Other) /*noexcept*/ = default; \
    ClassName& operator =(const ClassName& Other) /*noexcept*/ = default; \

#define DISABLE_MOVE(ClassName) \
    ClassName(ClassName&& Other) noexcept = delete; \
    ClassName& operator =(ClassName&& Other) noexcept = delete;

#define ENABLE_MOVE(ClassName) \
    ClassName(ClassName&& Other) noexcept = default; \
    ClassName& operator =(ClassName&& Other) noexcept = default;

#define ENABLE_MOVE_DECL(ClassName) \
    ClassName(ClassName&& Other) noexcept; \
    ClassName& operator =(ClassName&& Other) noexcept;

#define DISABLE_COPY_ENABLE_MOVE(ClassName) \
    DISABLE_COPY(ClassName); \
    ENABLE_MOVE(ClassName);

#define DISABLE_COPY_ENABLE_MOVE_DECL(ClassName) \
    DISABLE_COPY(ClassName); \
    ENABLE_MOVE_DECL(ClassName);

#define DISABLE_COPY_DISABLE_MOVE(ClassName) \
    DISABLE_COPY(ClassName); \
    DISABLE_MOVE(ClassName);

#define IMPEMENT_ENUM_BITFIELD(EnumType)                                                \
    inline EnumType operator | (EnumType a, EnumType b)                                 \
    {                                                                                   \
        return static_cast<EnumType>(static_cast<uint64>(a) | static_cast<uint64>(b));  \
    }                                                                                   \
                                                                                        \
    inline EnumType& operator |= (EnumType& a, EnumType b)                              \
    {                                                                                   \
        a = a | b;                                                                      \
        return a;                                                                       \
    }                                                                                   \
                                                                                        \
    inline uint64 operator & (EnumType a, EnumType b)                                   \
    {                                                                                   \
        return static_cast<uint64>(a) & static_cast<uint64>(b);                         \
    }                                                                                   \

#define ENABLE_DEBUG_RENDER_BACKEND 1

} // namespace Cyclone
