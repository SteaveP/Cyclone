#include "Profiling.h"

#include "Engine/Core/Helpers.h"
#include "Engine/Render/Backend/IRendererBackend.h"

#include "Engine/Render/Backend/CommandBuffer.h"
#include "Engine/Render/Backend/CommandQueue.h"

#include <cstdarg>

namespace Cyclone
{

constexpr uint32 NameBufferSize = 128;
namespace Render { class CCommandBuffer; }

C_STATUS ENGINE_API GInitProfiling()
{
    // Initialize all needed data here
    // but at the moment nothing to needs to be initialized yet
    return C_STATUS::C_STATUS_OK;
}

void ENGINE_API GDeInitProfiling()
{

}

CScopedGPUProfileEvent::CScopedGPUProfileEvent(Render::CCommandBuffer* CommandBuffer, Vec4 Color, const char* Format, ...)
{
    this->CommandBuffer = CommandBuffer;

    char Buf[NameBufferSize];

    va_list args;
    va_start(args, Format);
    vsnprintf(Buf, _countof(Buf), Format, args);
    va_end(args);

    GPUProfileEventBegin(CommandBuffer, Color, Buf);
}

CScopedGPUProfileEvent::CScopedGPUProfileEvent(Render::CCommandQueue* CommandQueue, Vec4 Color, const char* Format, ...)
{
    this->CommandQueue = CommandQueue;

    char Buf[NameBufferSize];

    va_list args;
    va_start(args, Format);
    vsnprintf(Buf, _countof(Buf), Format, args);
    va_end(args);

    GPUProfileEventBegin(CommandQueue, Color, Buf);
}

CScopedGPUProfileEvent::~CScopedGPUProfileEvent()
{
    if (CommandBuffer)
        GPUProfileEventEnd(CommandBuffer);

    if (CommandQueue)
        GPUProfileEventEnd(CommandQueue);
}

void CScopedGPUProfileEvent::GPUProfileEventBegin(Render::CCommandBuffer* CommandBuffer, Vec4 Color, const char* Format, ...)
{
    if (CommandBuffer == nullptr)
        return;

    char Buf[NameBufferSize];

    va_list args;
    va_start(args, Format);
    vsnprintf(Buf, _countof(Buf), Format, args);
    va_end(args);

    CommandBuffer->GetCommandQueue()->GetBackend()->ProfileGPUEventBegin(CommandBuffer, Buf, MoveTemp(Color));
}

void CScopedGPUProfileEvent::GPUProfileEventBegin(Render::CCommandQueue* CommandQueue, Vec4 Color, const char* Format, ...)
{
    if (CommandQueue == nullptr)
        return;

    char Buf[NameBufferSize];

    va_list args;
    va_start(args, Format);
    vsnprintf(Buf, _countof(Buf), Format, args);
    va_end(args);

    CommandQueue->GetBackend()->ProfileGPUEventBegin(CommandQueue, Buf, MoveTemp(Color));
}

void CScopedGPUProfileEvent::GPUProfileEventEnd(Render::CCommandBuffer* CommandBuffer)
{
    if (CommandBuffer)
        CommandBuffer->GetCommandQueue()->GetBackend()->ProfileGPUEventEnd(CommandBuffer);
}

void CScopedGPUProfileEvent::GPUProfileEventEnd(Render::CCommandQueue* CommandQueue)
{
    if (CommandQueue)
        CommandQueue->GetBackend()->ProfileGPUEventEnd(CommandQueue);
}

} //namespace Cyclone
