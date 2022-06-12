#pragma once

#include "Engine/Framework/IRenderer.h"
#include "Engine/Render/IRendererBackend.h"
#include "RenderBackendVkModule.h"

#include "Common/CommonVulkan.h"

#include "GlobalContextVulkan.h"
#include "WindowContextVulkan.h"

namespace Cyclone::Render
{

class RenderPassVk;
class FrameBufferVk;

class RenderBackendVulkan : public IRendererBackend
{
public:
    virtual C_STATUS Init(IRenderer* Renderer) override;
    virtual C_STATUS Shutdown() override;

    virtual C_STATUS BeginRender() override;
    virtual C_STATUS Render() override;
    virtual C_STATUS EndRender() override;

    virtual uint32_t GetCurrentFrame() const override { return m_CurrentFrame; }
    virtual uint32_t GetCurrentLocalFrame() const override { return m_CurrentLocalFrame; }

    GlobalContextVulkan& GetGlobalContext() { return m_GlobalContext; }
    WindowContextVulkan& GetWindowContext() { return m_WindowContext; }

    VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }

    VkImageView CreateImageView(VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspectMask);

    VkFormat FindSupportedFormat(DeviceHandle Device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat(DeviceHandle Device);
    bool hasStencilComponent(VkFormat format);

protected:
    IRenderer* m_Renderer = nullptr;

protected:
    GlobalContextVulkan m_GlobalContext;
    WindowContextVulkan m_WindowContext;

    VkDescriptorPool m_DescriptorPool;

    uint32_t m_CurrentLocalFrame = 0; // Local frame counter [0, MAX_FRAMES_IN_FLIGHT]
    uint32_t m_CurrentFrame = 0; // Global frame counter
};

} // namespace Cyclone::Render
