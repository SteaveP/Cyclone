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
    WindowContextVulkan& GetWindowContext(uint32 Index) { return m_WindowContexts[Index]; }
    uint32 GetWindowContextCount() const { return static_cast<uint32>(m_WindowContexts.size()); }

    VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }

    VkImageView CreateImageView(DeviceHandle Device, VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspectMask);

    VkFormat FindSupportedFormat(DeviceHandle Device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat(DeviceHandle Device);
    bool hasStencilComponent(VkFormat format);

protected:
    IRenderer* m_Renderer = nullptr;

protected:
    GlobalContextVulkan m_GlobalContext;
    std::vector<WindowContextVulkan> m_WindowContexts;

    VkDescriptorPool m_DescriptorPool;

    uint32_t m_CurrentLocalFrame = 0; // Local frame counter [0, MAX_FRAMES_IN_FLIGHT]
    uint32_t m_CurrentFrame = 0; // Global frame counter
};

} // namespace Cyclone::Render
