#pragma once

#include "Engine/Framework/IRenderer.h"
#include "Engine/Utils/Delegate.h"

#include "Engine/Render/Backend/IRendererBackend.h"
#include "Engine/Render/Utils/Pool.h"

#include "CommonVulkan.h"

#include "Internal/DeviceManagerVulkan.h"
#include "Internal/DisposalManagerVulkan.h"

namespace Cyclone::Render
{

class IResourceManager;
class CRenderBackendVulkan;
class CResourceManagerVulkan;
class CWindowContext;
class CDisposalManager;
class CBindlessDescriptorManagerVulkan;

class CRenderBackendVulkan : public IRendererBackend
{
public:
    DISABLE_COPY_ENABLE_MOVE(CRenderBackendVulkan);

    CRenderBackendVulkan();
    ~CRenderBackendVulkan();
    //////////////////////////////////////////////////////////////////////////
    // IRendererBackend methods
    //////////////////////////////////////////////////////////////////////////
    virtual C_STATUS Init(IRenderer* Renderer) override;
    virtual C_STATUS Shutdown() override;

    virtual C_STATUS BeginRender() override;
    virtual C_STATUS EndRender() override;

    virtual void WaitGPU() override;

    virtual IRenderer* GetRenderer() const override { return m_Renderer; }
    virtual IBindlessManager* GetBindlessManager(CDeviceHandle DeviceHandle) override { return GetBindlessManagerVk(DeviceHandle); }
    virtual IResourceManager* GetResourceManager(CDeviceHandle DeviceHandle) override { return GetResourceManagerVk(DeviceHandle); }
    virtual CUploadQueue* GetUploadQueue(CDeviceHandle DeviceHandle) override;

    virtual CHandle<CWindowContext> CreateWindowContext() override;
    virtual void DestroyWindowContext(CHandle<CWindowContext> Handle) override;
    virtual CWindowContext* GetWindowContext(CHandle<CWindowContext> Handle) override;

    virtual CDeviceDelegate* GetOnDeviceCreatedDelegate() override { return &m_OnDeviceCreatedDelegate; }
    virtual CDeviceDelegate* GetOnDeviceRemovedDelegate() override { return &m_OnDeviceRemovedDelegate; }

    virtual void ProfileGPUEventBegin(CCommandBuffer* CommandBuffer, const char* Name, const Vec4& Color) override;
    virtual void ProfileGPUEventEnd(CCommandBuffer* CommandBuffer) override;

    virtual void ProfileGPUEventBegin(CCommandQueue* CommandQueue, const char* Name, const Vec4& Color) override;
    virtual void ProfileGPUEventEnd(CCommandQueue* CommandQueue) override;
    
    //////////////////////////////////////////////////////////////////////////
    // Own methods
    //////////////////////////////////////////////////////////////////////////
    VkFormat FindSupportedFormat(CDeviceHandle Device, const Vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat(CDeviceHandle Device);
    bool HasStencilComponent(VkFormat format); // #todo_vk_stencil

    CDeviceManagerVulkan& GetDeviceManager() { return m_DeviceManager; }
    const CDeviceManagerVulkan& GetDeviceManager() const { return m_DeviceManager; }
    CResourceManagerVulkan* GetResourceManagerVk(CDeviceHandle DeviceHandle);
    CDisposalManagerVulkan* GetDisposalManagerVk(CDeviceHandle DeviceHandle);

    CBindlessDescriptorManagerVulkan* GetBindlessManagerVk(CDeviceHandle Device);

private:
    void ShutdownImpl() noexcept;

protected:
    IRenderer* m_Renderer = nullptr;

    CDeviceManagerVulkan m_DeviceManager;

    CDeviceDelegate m_OnDeviceCreatedDelegate;
    CDeviceDelegate m_OnDeviceRemovedDelegate;

    CPool<CWindowContextVulkan, CWindowContext> m_WindowContextPool;
};

} // namespace Cyclone::Render
