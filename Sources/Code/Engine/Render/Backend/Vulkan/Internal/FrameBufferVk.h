#pragma once

#include "CommonVulkan.h"

namespace Cyclone::Render
{

class RenderBackendVulkan;
class RenderPassVk;

struct FrameBufferVkInitInfo
{
    DISABLE_COPY_DISABLE_MOVE(FrameBufferVkInitInfo);
    FrameBufferVkInitInfo() = default;

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
    ~FrameBufferVk();

    C_STATUS Init(const FrameBufferVkInitInfo& InitInfo);
    void DeInit();

    VkFramebuffer Get() const { return m_FrameBuffer; }
    RenderPassVk* GetRenderPass() const { return m_RenderPass; }

protected:
    RenderPassVk* m_RenderPass = nullptr;
    VkFramebuffer m_FrameBuffer = VK_NULL_HANDLE;
};

} // namespace Cyclone::Render
