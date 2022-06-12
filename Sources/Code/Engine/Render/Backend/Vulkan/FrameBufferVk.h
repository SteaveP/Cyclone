#pragma once

#include "Common/CommonVulkan.h"

namespace Cyclone::Render
{

class RenderBackendVulkan;
class RenderPassVk;

struct FrameBufferVkInitInfo
{
    std::array<VkImageView, 10> Attachments;
    uint32_t AttachmentsCount;

    uint32_t Width;
    uint32_t Height;
    uint32_t Layers;

    DeviceHandle Device;

    RenderPassVk* RenderPass;
    RenderBackendVulkan* Backend;
};

class FrameBufferVk
{
public:
    C_STATUS FrameBufferVk::Init(const FrameBufferVkInitInfo& InitInfo);

    VkFramebuffer Get() const { return m_frameBuffer; }
    RenderPassVk* GetRenderPass() const { return m_renderPass; }

protected:
    RenderPassVk* m_renderPass = nullptr;
    VkFramebuffer m_frameBuffer = VK_NULL_HANDLE;
};

} // namespace Cyclone::Render
