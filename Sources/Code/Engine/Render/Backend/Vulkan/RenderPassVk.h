#pragma once

#include "Common/CommonVulkan.h"

#include <array>

#define RENDER_PASS_MAX_ATTACHMENTS 8

namespace Cyclone::Render
{

class RenderBackendVulkan;
class CommandBufferVk;
class FrameBufferVk;

struct RenderPassVkInitInfo
{
    uint32_t ColorAttachmentCount;
    std::array<VkAttachmentDescription, RENDER_PASS_MAX_ATTACHMENTS> ColorAttachment;
    uint32_t ColorAttachmentRefCount;
    std::array<VkAttachmentReference, RENDER_PASS_MAX_ATTACHMENTS> ColorAttachmentRef;

    bool UseDepth;
    VkAttachmentDescription DepthAttachment;
    VkAttachmentReference DepthAttachmentRef;

    DeviceHandle Device;

    uint32_t SubpassCount;
    std::array<VkSubpassDescription, RENDER_PASS_MAX_ATTACHMENTS> Subpass;
    uint32_t SubpassDependencyCount;
    std::array<VkSubpassDependency, RENDER_PASS_MAX_ATTACHMENTS> SubpassDependency;

    RenderBackendVulkan* Backend;
};

struct RenderPassVkBeginInfo
{
    FrameBufferVk* FrameBuffer;
    VkRect2D RenderArea;
    std::array<VkClearValue, 10> ClearColors;
    uint32_t ClearColorsCount;
};

class RenderPassVk
{
public:
    C_STATUS Init(const RenderPassVkInitInfo& InitInfo);

    void Begin(CommandBufferVk* CommandBuffer, const RenderPassVkBeginInfo& Info);
    void End(CommandBufferVk* CommandBuffer);

    VkRenderPass Get() const { return m_renderPass; }

protected:
    RenderBackendVulkan* m_Backend = nullptr;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
};

} // namespace Cyclone::Render
