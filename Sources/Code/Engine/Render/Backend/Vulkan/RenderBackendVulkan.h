#pragma once

#include "Engine/Framework/IRenderer.h"
#include "Engine/Render/IRendererBackend.h"

#include "CommonVulkan.h"

#include "Internal/GlobalContextVk.h"
#include "WindowContextVulkan.h"

namespace Cyclone::Render
{

class CResourceManagerVk;

class RenderBackendVulkan : public IRendererBackend
{
public:
    virtual C_STATUS Init(IRenderer* Renderer) override;
    virtual C_STATUS Shutdown() override;

    virtual C_STATUS BeginRender() override;
    virtual C_STATUS EndRender() override;

    virtual void WaitGPU() override;

    virtual IRenderer* GetRenderer() const override { return m_Renderer; }

    virtual CWindowContext* CreateWindowContext(IWindow* Window) override;
    virtual CCommandQueue* CreateCommandQueue() override;
    virtual CCommandBuffer* CreateCommandBuffer() override;
    virtual CShader* CreateShader() override;
    virtual CPipeline* CreatePipeline() override;

    virtual CTexture* CreateTexture() override;
    virtual RawPtr CreateTextureView(CDeviceHandle Device, CTexture* Texture, uint32 StartMip, uint32 MipLevels, EFormatType Format, EImageAspectType AspectMask) override;
    virtual void DestroyTextureView(CDeviceHandle Device, CTexture* Texture, RawPtr TextureView);

    virtual CBuffer* CreateBuffer() override;
    virtual RawPtr CreateBufferView(CDeviceHandle Device, CBuffer* Buffer, uint64 Offset, uint64 Size, EFormatType Format) override;
    virtual void DestroyBufferView(CDeviceHandle Device, CBuffer* Buffer, RawPtr BufferView);

    VkImageView CreateImageView(CDeviceHandle Device, VkImage Image, uint32 MipLevels, VkFormat Format, VkImageAspectFlags AspectMask);
    VkBufferView CreateBufferView(CDeviceHandle Device, VkBuffer Buffer, uint64 Offset, uint64 Size, VkFormat Format);

    VkFormat FindSupportedFormat(CDeviceHandle Device, const Vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat(CDeviceHandle Device);
    bool HasStencilComponent(VkFormat format);

    GlobalContextVulkan& GetGlobalContext() { return m_GlobalContext; }
    CResourceManagerVk* GetResourceManager(CDeviceHandle Device);

protected:
    IRenderer* m_Renderer = nullptr;

protected:
    GlobalContextVulkan m_GlobalContext;
};

} // namespace Cyclone::Render
