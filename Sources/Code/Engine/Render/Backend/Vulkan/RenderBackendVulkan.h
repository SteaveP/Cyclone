#pragma once

#include "Engine/Framework/IRenderer.h"
#include "Engine/Render/IRendererBackend.h"

#include "CommonVulkan.h"

#include "Internal/GlobalContextVulkan.h"
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
    virtual C_STATUS EndRender() override;

    virtual void WaitGPU() override;

    GlobalContextVulkan& GetGlobalContext() { return m_GlobalContext; }

    VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }

    VkImageView CreateImageView(DeviceHandle Device, VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspectMask);

    VkFormat FindSupportedFormat(DeviceHandle Device, const Vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat(DeviceHandle Device);
    bool HasStencilComponent(VkFormat format);

    virtual IRenderer* GetRenderer() const override { return m_Renderer; }

    virtual CWindowContext* CreateWindowContext(IWindow* Window) override;
    virtual CCommandQueue* CreateCommandQueue() override;
    virtual CCommandBuffer* CreateCommandBuffer() override;
    virtual CTexture* CreateTexture() override;

protected:
    IRenderer* m_Renderer = nullptr;

protected:
    GlobalContextVulkan m_GlobalContext;

    VkDescriptorPool m_DescriptorPool; // #todo_move remove

};

} // namespace Cyclone::Render
