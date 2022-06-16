#pragma once

#include "CommonVulkan.h"

namespace Cyclone::Render
{

class FrameBufferVk;

class RenderPassVk
{
public:
    ~RenderPassVk();

    C_STATUS Init(RenderBackendVulkan* Backend, CDeviceHandle DeviceHandle, const CRenderPass& RenderPass);
    void DeInit();

    void Begin(CommandBufferVulkan* CommandBuffer, FrameBufferVk* FrameBuffer, const CRenderPass& RenderPass);
    void End(CommandBufferVulkan* CommandBuffer);

    VkRenderPass Get() const { return m_RenderPass; }
    RenderBackendVulkan* GetBackend() const { return m_Backend; }
    CDeviceHandle GetDevice() const { return m_Device; }
    const FrameBufferVk* GetFrameBuffer() const { return m_FrameBuffer; }
    VkRect2D GetRenderArea() const { return m_Viewport; }
    uint32 GetColorAttachmentCount() const { return m_ColorAttachmentCount; }

protected:
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    FrameBufferVk* m_FrameBuffer = VK_NULL_HANDLE;
    VkRect2D m_Viewport;
    uint32 m_ColorAttachmentCount = 0;

    RenderBackendVulkan* m_Backend = nullptr;
    CDeviceHandle m_Device;
};

} // namespace Cyclone::Render
