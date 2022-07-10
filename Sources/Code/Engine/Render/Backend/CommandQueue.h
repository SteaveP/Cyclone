#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Framework/IRenderer.h"

#include "Engine/Render/CommonRender.h"

namespace Cyclone::Render
{

class IRendererBackend;
class CCommandBuffer;

class ENGINE_API CCommandQueue
{
public:
    DISABLE_COPY_ENABLE_MOVE(CCommandQueue);

    CCommandQueue();
    virtual ~CCommandQueue();

    virtual void DeInit();

    virtual C_STATUS OnBeginRender();
    virtual C_STATUS OnEndRender();

    virtual CCommandBuffer* AllocateCommandBuffer();
    virtual void ReturnCommandBuffer(CCommandBuffer* CommandBuffer);

    virtual CommandQueueType GetCommandQueueType() const = 0;

    virtual C_STATUS Submit(CCommandBuffer** CommandBuffers, uint32_t CommandBuffersCount, bool AutoReturnToPool = true);

    IRendererBackend* GetBackend() { return m_Backend; }
    const IRendererBackend* GetBackend() const { return m_Backend; }
    CDeviceHandle GetDeviceHandle() const { return m_DeviceHandle; }

private:
    void DeInitImpl() noexcept;

protected:
    CDeviceHandle m_DeviceHandle;
    IRendererBackend* m_Backend = nullptr;
};

} // namespace Cyclone::Render
