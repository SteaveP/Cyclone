#pragma once

namespace Cyclone
{

#define PROFILING_TOOL_DISABLE 0
#define PROFILING_TOOL_PIX 1

#define PROFILING_TOOL PROFILING_TOOL_DISABLE 
    //PROFILING_TOOL_PIX

#if PROFILING_TOOL == PROFILING_TOOL_PIX

#include "ThirdpartyIntegration/PIX/PIX.h"
#define PROFILE_COLOR_DEFAULT PIX_PROFILE_COLOR_DEFAULT

#define PROFILE_GPU_SCOPED_EVENT(commandListOrQueue, formatString, ...) PIX_PROFILE_GPU_SCOPED_EVENT(commandListOrQueue, formatString, __VA_ARGS__)
#define PROFILE_GPU_SCOPED_EVENT_COLOR(commandListOrQueue, color, formatString, ...) PIX_PROFILE_GPU_SCOPED_EVENT_COLOR(commandListOrQueue, color, formatString, __VA_ARGS__)

#define PROFILE_CPU_SCOPED_EVENT(formatString, ...) PIX_PROFILE_CPU_SCOPED_EVENT(formatString, __VA_ARGS__)
#define PROFILE_CPU_SCOPED_EVENT_COLOR(color, formatString, ...) PIX_PROFILE_CPU_SCOPED_EVENT_COLOR(color, formatString, __VA_ARGS__)

#define PROFILE_GPU_MARKER(commandListOrQueue, formatString, ...) PIX_PROFILE_GPU_MARKER(commandListOrQueue, formatString, __VA_ARGS__)
#define PROFILE_GPU_MARKER_COLOR(commandListOrQueue, color, formatString, ...) PIX_PROFILE_GPU_MARKER_COLOR(commandListOrQueue, color, formatString, __VA_ARGS__)

#define PROFILE_CPU_MARKER(formatString, ...) PIX_PROFILE_CPU_MARKER(formatString, __VA_ARGS__)
#define PROFILE_CPU_MARKER_COLOR(color, formatString, ...) PIX_PROFILE_CPU_MARKER_COLOR(color, formatString, __VA_ARGS__)


#define PROFILE_GPU_BEGIN_EVENT(commandListOrQueue, formatString, ...) PIX_PROFILE_GPU_BEGIN_EVENT(commandListOrQueue, formatString, __VA_ARGS__)
#define PROFILE_GPU_BEGIN_EVENT_COLOR(commandListOrQueue, color, formatString, ...) PIX_PROFILE_GPU_BEGIN_EVENT_COLOR(commandListOrQueue, color, formatString, __VA_ARGS__)

#define PROFILE_CPU_BEGIN_EVENT(formatString, ...) PIX_PROFILE_CPU_BEGIN_EVENT(formatString, __VA_ARGS__)
#define PROFILE_CPU_BEGIN_EVENT_COLOR(color, formatString, ...) PIX_PROFILE_CPU_BEGIN_EVENT_COLOR(color, formatString, __VA_ARGS__)

#define PROFILE_CPU_END_EVENT() PIX_PROFILE_CPU_END_EVENT()
#define PROFILE_GPU_END_EVENT(commandListOrQueue) PIX_PROFILE_GPU_END_EVENT(commandListOrQueue)

#else

#define PROFILE_COLOR_DEFAULT
#define PROFILE_GPU_SCOPED_EVENT_COLOR(commandListOrQueue, color, formatString, ...)
#define PROFILE_GPU_SCOPED_EVENT(commandListOrQueue, formatString, ...)
#define PROFILE_CPU_SCOPED_EVENT_COLOR(color, formatString, ...)
#define PROFILE_CPU_SCOPED_EVENT(formatString, ...)

#define PROFILE_GPU_BEGIN_EVENT_COLOR(commandListOrQueue, color, formatString, ...)
#define PROFILE_GPU_BEGIN_EVENT(commandListOrQueue, formatString, ...)
#define PROFILE_CPU_BEGIN_EVENT_COLOR(color, formatString, ...)
#define PROFILE_CPU_BEGIN_EVENT(formatString, ...)

#define PROFILE_CPU_END_EVENT()
#define PROFILE_GPU_END_EVENT(commandListOrQueue)

#endif

} // namespace Cyclone