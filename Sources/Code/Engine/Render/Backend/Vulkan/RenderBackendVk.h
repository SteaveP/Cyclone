#pragma once

#include "Engine/Framework/IRenderer.h"
#include "Engine/Render/IRendererBackend.h"
#include "RenderBackendVkModule.h"

#include "Common/CommonVulkan.h"

#include "GlobalContextVk.h"
#include "WindowContextVk.h"

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

    virtual uint32_t GetCurrentFrame() const override { return m_currentFrame; }
    virtual uint32_t GetCurrentLocalFrame() const override { return m_currentLocalFrame; }

    GlobalContextVk& GetGlobalContext() { return m_globalContext; }
    WindowContextVk& GetWindowContext() { return m_windowContext; }

    VkDescriptorPool GetDescriptorPool() const { return m_descriptorPool; }

    VkImageView CreateImageView(VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspectMask);

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat();
    bool hasStencilComponent(VkFormat format);

protected:
    IRenderer* m_renderer = nullptr;

protected:
    GlobalContextVk m_globalContext;
    WindowContextVk m_windowContext;

    VkDescriptorPool m_descriptorPool;

    uint32_t m_currentLocalFrame = 0; // Local frame counter [0, MAX_FRAMES_IN_FLIGHT]
    uint32_t m_currentFrame = 0; // Global frame counter
};

} // namespace Cyclone::Render
