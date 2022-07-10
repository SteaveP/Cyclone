#pragma once

#include "CommonVulkan.h"
#include "Engine/Render/Backend/WindowContext.h"

namespace Cyclone::Render
{

class CWindowContextVulkan : public CWindowContext
{
protected:
    static const uint32 MAX_SWAPCHAIN_IMAGE_COUNT = 10;

public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CWindowContextVulkan);

    CWindowContextVulkan();
    virtual ~CWindowContextVulkan();

    virtual C_STATUS Init(IRenderer* Renderer, IWindow* Window) override;
    virtual C_STATUS DeInit() override;

    virtual C_STATUS BeginRender() override;
    virtual C_STATUS Present() override;

    virtual C_STATUS OnWindowResize() override { return RecreateSwapchain(); };

    virtual CCommandQueue* GetCommandQueue(CommandQueueType QueueType) const override;

    CRenderBackendVulkan* GetBackend() const { return m_Backend; }
    IWindow* GetWindow() const { return m_Window; }

    VkFormat GetSwapchainImageFormat() const { return m_SwapchainImageFormat; }
    VkExtent2D GetSwapchainExtent() const { return m_SwapchainExtent; }
    uint32 GetMinSwapchainImageCount() const { return m_MinSwapchainImageCount; }

    VkSemaphore GetImageAvailableSemaphore(uint32 Index) const { return m_ImageAvailabeSemaphores[Index]; }
    VkFence GetInflightFence(uint32 Index) const { return m_InflightFences[Index]; }

    VkSampleCountFlagBits GetCurrentMsaaSamples() const {return m_CurrentMsaaSamples; }

    CCommandQueueVulkan* GetCommandQueueVk(CommandQueueType QueueType) { return m_CommandQueueCache[(uint32)QueueType]; }

protected:
    C_STATUS CreateSurface(IWindow* Window);
    void DestroySurface(bool IsDeferred = false);

    C_STATUS CreateSwapchain();
    C_STATUS CreateSwapchainImageViews();
    void CleanupSwapchain(bool IsDeferred = false, bool IsRecreating = false);

    C_STATUS CreateSyncObjects();
    void DestroySyncObjects(bool IsDeferred = false);

    C_STATUS RecreateSwapchain();

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const Vector<VkSurfaceFormatKHR>& AvailableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const Vector<VkPresentModeKHR>& AvailablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);

private:
    void DeInitImpl() noexcept;

protected:
    CRenderBackendVulkan* m_Backend = nullptr;

    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    
    uint32 m_MinSwapchainImageCount = 0;
    Vector<VkImage> m_SwapchainImages;
    VkFormat m_SwapchainImageFormat;
    VkExtent2D m_SwapchainExtent;

    bool m_FramebufferResized = false;

    VkSemaphore m_ImageAvailabeSemaphores[MAX_SWAPCHAIN_IMAGE_COUNT]{};
    VkSemaphore m_RenderFinishedSemaphores[MAX_SWAPCHAIN_IMAGE_COUNT]{};
    VkFence m_InflightFences[MAX_SWAPCHAIN_IMAGE_COUNT]{};
    VkFence m_ImagesInFlight[MAX_SWAPCHAIN_IMAGE_COUNT]{};

    CCommandQueueVulkan* m_CommandQueueCache[(uint32)CommandQueueType::Count]{};

    VkSampleCountFlagBits m_CurrentMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
};

} // namespace Cyclone::Render
