#pragma once

#include "CommonVulkan.h"

#define RENDER_PASS_MAX_ATTACHMENTS 10

namespace Cyclone::Render
{

struct RenderPassVkInitInfo
{
    DISABLE_COPY_DISABLE_MOVE(RenderPassVkInitInfo);
    RenderPassVkInitInfo() = default;

    uint32_t ColorAttachmentCount = 0;
    Array<VkAttachmentDescription, RENDER_PASS_MAX_ATTACHMENTS> ColorAttachment;
    uint32_t ColorAttachmentRefCount = 0;
    Array<VkAttachmentReference, RENDER_PASS_MAX_ATTACHMENTS> ColorAttachmentRef;

    bool UseDepth = false;
    VkAttachmentDescription DepthAttachment;
    VkAttachmentReference DepthAttachmentRef;

    uint32_t SubpassCount = 0;
    Array<VkSubpassDescription, RENDER_PASS_MAX_ATTACHMENTS> Subpass;
    uint32_t SubpassDependencyCount = 0;
    Array<VkSubpassDependency, RENDER_PASS_MAX_ATTACHMENTS> SubpassDependency;

    const CRenderPass* SourceRenderPass = nullptr;
    RenderBackendVulkan* Backend = nullptr;

    DeviceHandle Device;

    static void CreateFromRenderPass(RenderPassVkInitInfo& Desc, RenderBackendVulkan* Backend, DeviceHandle Handle, const CRenderPass& RenderPass);
};

struct RenderPassVkBeginInfo
{
    FrameBufferVk* FrameBuffer = nullptr;
    VkRect2D RenderArea;
    Array<VkClearValue, 10> ClearColors;
    uint32_t ClearColorsCount = 0;
};

class RenderPassVk
{
public:
    ~RenderPassVk();

    C_STATUS Init(const RenderPassVkInitInfo& InitInfo);
    void DeInit();

    void Begin(CommandBufferVulkan* CommandBuffer, const RenderPassVkBeginInfo& Info);
    void End(CommandBufferVulkan* CommandBuffer);

    VkRenderPass Get() const { return m_RenderPass; }
    RenderBackendVulkan* GetBackend() const { return m_Backend; }
    DeviceHandle GetDevice() const { return m_Device; }

protected:
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;

    RenderBackendVulkan* m_Backend = nullptr;
    const CRenderPass* m_SourceRenderPass = nullptr;
    DeviceHandle m_Device;
};

} // namespace Cyclone::Render
