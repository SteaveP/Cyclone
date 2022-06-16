#pragma once

#include "CommonVulkan.h"

namespace Cyclone::Render
{

class RenderBackendVulkan;
class RenderPassVk;

class FrameBufferVk
{
public:
    ~FrameBufferVk();

    C_STATUS Init(CDeviceHandle DeviceHandle, RenderBackendVulkan* BackendVk, RenderPassVk* RenderPassVkPtr, const CRenderPass& RenderPass);
    void DeInit();

    VkFramebuffer Get() const { return m_FrameBuffer; }
    RenderPassVk* GetRenderPass() const { return m_RenderPass; }

//     uint32 GetWidth() const { return m_Width; }
//     uint32 GetHeight() const { return m_Height; }
//     uint32 GetLayers() const { return m_Layers; }

protected:
    RenderPassVk* m_RenderPass = nullptr;
    VkFramebuffer m_FrameBuffer = VK_NULL_HANDLE;

//     uint32 m_Width;
//     uint32 m_Height;
//     uint32 m_Layers;
};

} // namespace Cyclone::Render
