#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Math.h"

namespace Cyclone
{

C_STATUS ENGINE_API GInitProfiling();
void ENGINE_API GDeInitProfiling();

#define PROFILING_TOOL_GPU_DISABLE 0
#define PROFILING_TOOL_GPU_PIX 1
#define PROFILING_TOOL_GPU_FROM_RENDER_BACKEND 2

#define PROFILING_TOOL_CPU_DISABLE 0
#define PROFILING_TOOL_CPU_PIX 1
#define PROFILING_TOOL_CPU_OPTICK 2

#define PROFILING_TOOL_CPU PROFILING_TOOL_CPU_OPTICK
#define PROFILING_TOOL_GPU PROFILING_TOOL_GPU_FROM_RENDER_BACKEND

namespace Render { class CCommandBuffer; class CCommandQueue; }


struct CScopedGPUProfileEvent
{
    CScopedGPUProfileEvent(Render::CCommandBuffer* CommandBuffer, Vec4 Color, const char* Format, ...);
    CScopedGPUProfileEvent(Render::CCommandQueue* CommandQueue, Vec4 Color, const char* Format, ...);
    ~CScopedGPUProfileEvent();

    Render::CCommandBuffer* CommandBuffer = nullptr;
    Render::CCommandQueue* CommandQueue = nullptr;

    static void GPUProfileEventBegin(Render::CCommandBuffer* CommandBuffer, Vec4 Color, const char* Format, ...);
    static void GPUProfileEventBegin(Render::CCommandQueue* CommandQueue, Vec4 Color, const char* Format, ...);

    static void GPUProfileEventEnd(Render::CCommandBuffer* CommandBuffer);
    static void GPUProfileEventEnd(Render::CCommandQueue* CommandQueue);
};

} //namespace Cyclone

#if PROFILING_TOOL_GPU == PROFILING_TOOL_GPU_PIX

#include "ThirdpartyIntegration/PIX/PIX.h"
#ifndef PROFILE_COLOR_DEFAULT
#define PROFILE_COLOR_DEFAULT PIX_PROFILE_COLOR_DEFAULT
#endif

#define PROFILE_GPU_SCOPED_EVENT(CommandListOrQueue, FormatString, ...) PIX_PROFILE_GPU_SCOPED_EVENT(CommandListOrQueue, FormatString, __VA_ARGS__)
#define PROFILE_GPU_SCOPED_EVENT_COLOR(CommandListOrQueue, Color, FormatString, ...) PIX_PROFILE_GPU_SCOPED_EVENT_COLOR(CommandListOrQueue, Color, FormatString, __VA_ARGS__)

#define PROFILE_GPU_MARKER(CommandListOrQueue, FormatString, ...) PIX_PROFILE_GPU_MARKER(CommandListOrQueue, FormatString, __VA_ARGS__)
#define PROFILE_GPU_MARKER_COLOR(CommandListOrQueue, Color, FormatString, ...) PIX_PROFILE_GPU_MARKER_COLOR(CommandListOrQueue, Color, FormatString, __VA_ARGS__)

#define PROFILE_GPU_BEGIN_EVENT(CommandListOrQueue, FormatString, ...) PIX_PROFILE_GPU_BEGIN_EVENT(CommandListOrQueue, FormatString, __VA_ARGS__)
#define PROFILE_GPU_BEGIN_EVENT_COLOR(CommandListOrQueue, Color, FormatString, ...) PIX_PROFILE_GPU_BEGIN_EVENT_COLOR(CommandListOrQueue, Color, FormatString, __VA_ARGS__)

#define PROFILE_GPU_END_EVENT(CommandListOrQueue) PIX_PROFILE_GPU_END_EVENT(CommandListOrQueue)

#elif PROFILING_TOOL_GPU == PROFILING_TOOL_GPU_FROM_RENDER_BACKEND

#define PROFILE_COLOR_DEFAULT Vec4{1.f, 1.f, 1.f, 0.f}

#define PROFILE_GPU_SCOPED_EVENT(CommandListOrQueue, FormatString, ...) Cyclone::CScopedGPUProfileEvent ScopedGPUProfileEvent##__FILE##__LINE__(CommandListOrQueue, PROFILE_COLOR_DEFAULT, FormatString, __VA_ARGS__)
#define PROFILE_GPU_SCOPED_EVENT_COLOR(CommandListOrQueue, Color, FormatString, ...) Cyclone::CScopedGPUProfileEvent ScopedGPUProfileEvent##__FILE##__LINE__(CommandListOrQueue, Color, FormatString, __VA_ARGS__)

#define PROFILE_GPU_MARKER(CommandListOrQueue, FormatString, ...) FIXME
#define PROFILE_GPU_MARKER_COLOR(CommandListOrQueue, Color, FormatString, ...) FIXME

#define PROFILE_GPU_BEGIN_EVENT(CommandListOrQueue, FormatString, ...) Cyclone::CScopedGPUProfileEvent::GPUProfileEventBegin(CommandListOrQueue, PROFILE_COLOR_DEFAULT, FormatString, __VA_ARGS__)
#define PROFILE_GPU_BEGIN_EVENT_COLOR(CommandListOrQueue, Color, FormatString, ...) Cyclone::CScopedGPUProfileEvent::GPUProfileEventBegin(CommandListOrQueue, Color, FormatString, __VA_ARGS__)

#define PROFILE_GPU_END_EVENT(CommandListOrQueue) Cyclone::CScopedGPUProfileEvent::GPUProfileEventEnd(CommandListOrQueue)

#else // Disable

#define PROFILE_COLOR_DEFAULT
#define PROFILE_GPU_SCOPED_EVENT_COLOR(Color, FormatString, ...)
#define PROFILE_GPU_SCOPED_EVENT(FormatString, ...)

#define PROFILE_GPU_BEGIN_EVENT_COLOR(Color, FormatString, ...)
#define PROFILE_GPU_BEGIN_EVENT(FormatString, ...)

#define PROFILE_GPU_END_EVENT(CommandListOrQueue)

#endif // GPU

#if PROFILING_TOOL_CPU == PROFILING_TOOL_CPU_PIX

#include "ThirdpartyIntegration/PIX/PIX.h"
#ifndef PROFILE_COLOR_DEFAULT
#define PROFILE_COLOR_DEFAULT PIX_PROFILE_COLOR_DEFAULT
#endif

#define PROFILE_CPU_SCOPED_EVENT(FormatString, ...) PIX_PROFILE_CPU_SCOPED_EVENT(FormatString, __VA_ARGS__)
#define PROFILE_CPU_SCOPED_EVENT_COLOR(Color, FormatString, ...) PIX_PROFILE_CPU_SCOPED_EVENT_COLOR(Color, FormatString, __VA_ARGS__)

#define PROFILE_CPU_MARKER(FormatString, ...) PIX_PROFILE_CPU_MARKER(FormatString, __VA_ARGS__)
#define PROFILE_CPU_MARKER_COLOR(Color, FormatString, ...) PIX_PROFILE_CPU_MARKER_COLOR(Color, FormatString, __VA_ARGS__)

#define PROFILE_CPU_BEGIN_EVENT(FormatString, ...) PIX_PROFILE_CPU_BEGIN_EVENT(FormatString, __VA_ARGS__)
#define PROFILE_CPU_BEGIN_EVENT_COLOR(Color, FormatString, ...) PIX_PROFILE_CPU_BEGIN_EVENT_COLOR(Color, FormatString, __VA_ARGS__)

//#define PROFILE_CPU_END_EVENT() PIX_PROFILE_CPU_END_EVENT()

#elif PROFILING_TOOL_CPU == PROFILING_TOOL_CPU_OPTICK

#include "Engine/Modules/Profiling/OptickProfiler.h"
#ifndef PROFILE_COLOR_DEFAULT
#define PROFILE_COLOR_DEFAULT PIX_PROFILE_COLOR_DEFAULT
#endif

#define PROFILE_CPU_SCOPED_EVENT(FormatString, ...)
#define PROFILE_CPU_SCOPED_EVENT_COLOR(Color, FormatString, ...)

#define PROFILE_CPU_MARKER(FormatString, ...)
#define PROFILE_CPU_MARKER_COLOR(Color, FormatString, ...)

#define PROFILE_CPU_BEGIN_EVENT(FormatString, ...)
#define PROFILE_CPU_BEGIN_EVENT_COLOR(Color, FormatString, ...)

//#define PROFILE_CPU_END_EVENT() PIX_PROFILE_CPU_END_EVENT()

#else // Disable

#define PROFILE_COLOR_DEFAULT
#define PROFILE_CPU_SCOPED_EVENT_COLOR(Color, FormatString, ...)
#define PROFILE_CPU_SCOPED_EVENT(FormatString, ...)

#define PROFILE_CPU_BEGIN_EVENT_COLOR(Color, FormatString, ...)
#define PROFILE_CPU_BEGIN_EVENT(FormatString, ...)

//#define PROFILE_CPU_END_EVENT()

#endif // CPU
