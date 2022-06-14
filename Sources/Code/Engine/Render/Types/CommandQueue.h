#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Framework/IRenderer.h"

#include "Engine/Render/Common.h"

namespace Cyclone::Render
{

class IRendererBackend;
class CCommandBuffer;

class ENGINE_API CommandQueueDesc
{
public:
    IRenderer* Renderer;
    CommandQueueType QueueType;

    // #todo_vk
    //IRendererBackend* Backend, DeviceHandle Handle, CommandQueueType QueueType, uint32_t QueueFamilyIndex, uint32_t QueueIndex;
};

class ENGINE_API CCommandQueue
{
public:
    DISABLE_COPY_ENABLE_MOVE(CCommandQueue);

    CCommandQueue();
    virtual ~CCommandQueue();

    virtual C_STATUS Init(CommandQueueDesc Desc);
    virtual void DeInit();

    virtual C_STATUS OnBeginRender();
    virtual C_STATUS OnEndRender();

    virtual CCommandBuffer* AllocateCommandBuffer();
    virtual void ReturnCommandBuffer(CCommandBuffer* CommandBuffer);

    virtual C_STATUS Submit(CCommandBuffer** CommandBuffers, uint32_t CommandBuffersCount, bool AutoReturnToPool = true);

    virtual IRendererBackend* GetBackend() const { return m_Backend; }

    CommandQueueType GetCommandQueueType() const { return m_Desc.QueueType; }
    const CommandQueueDesc& GetDesc() const { return m_Desc; }

protected:
    CommandQueueDesc m_Desc;
    IRendererBackend* m_Backend;
};

} // namespace Cyclone::Render
