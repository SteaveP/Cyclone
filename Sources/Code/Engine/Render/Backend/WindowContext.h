#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"
#include "Engine/Render/CommonRender.h"

namespace Cyclone
{
class IRenderer;
class IWindow;
}

namespace Cyclone::Render
{

class CCommandQueue;

class ENGINE_API CWindowContext
{
public:
    DISABLE_COPY_ENABLE_MOVE(CWindowContext);

    CWindowContext();
    virtual ~CWindowContext();

    virtual C_STATUS Init(IRenderer* Renderer, IWindow* Window);
    virtual C_STATUS DeInit();

    virtual C_STATUS BeginRender();
    virtual C_STATUS Present();

    virtual C_STATUS OnWindowResize() = 0;

    virtual CCommandQueue* GetCommandQueue(CommandQueueType QueueType) const { return nullptr; }

    uint32 GetBackBuffersCount() const { return static_cast<uint32>(m_BackBuffers.size());  }
    const CRenderTarget& GetBackBuffer(uint32 Index) const { return m_BackBuffers[Index]; }
    const CRenderTarget& GetCurrentBackBuffer() const { return m_BackBuffers[GetCurrentImageIndex()]; }

    IRenderer* GetRenderer() const { return m_Renderer; }
    IWindow* GetWindow() const { return m_Window; }

    uint32 GetCurrentLocalFrame() const { return m_CurrentLocalFrame; }
    uint32 GetCurrentImageIndex() const { return m_CurrentImageIndex; }

    CDeviceHandle GetDeviceHandle() const { return m_DeviceHandle; }

private:
    void DeInitImpl() noexcept;

protected:
    IRenderer* m_Renderer = nullptr;
    IWindow* m_Window = nullptr;

    CDeviceHandle m_DeviceHandle;

    uint32 m_CurrentLocalFrame = 0;
    uint32 m_CurrentImageIndex = 0;

    Vector<CRenderTarget> m_BackBuffers;

};

} // namespace Cyclone::Render
