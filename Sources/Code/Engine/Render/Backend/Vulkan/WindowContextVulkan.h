#pragma once

#include "Common/CommonVulkan.h"

namespace Cyclone
{
class IWindow;
} // namespace Cyclone

namespace Cyclone::Render
{

class WindowContextVulkan
{
protected:
    static const uint32 MAX_SWAPCHAIN_IMAGE_COUNT = 10;
public:
    WindowContextVulkan();
    ~WindowContextVulkan();
    C_STATUS Init(RenderBackendVulkan* RenderBackend, IWindow* Window);
    C_STATUS Shutdown();

    C_STATUS BeginRender();
    C_STATUS Present(VkSemaphore RenderFinishedSemaphore);

    RenderBackendVulkan* GetBackend() const { return m_Backend; }
    IWindow* GetWindow() const { return m_Window; }

    VkPhysicalDevice GetPhysDevice() { return m_PhysDeviceHandleCache; }
    VkDevice GetDevice() { return m_DeviceHandleCache; }

    VkFormat GetSwapchainImageFormat() const { return m_SwapchainImageFormat; }
    VkExtent2D GetSwapchainExtent() const { return m_SwapchainExtent; }
    VkImageView GetSwapchainImageView(uint32 Index) const { return m_SwapchainImageViews[Index]; }
    uint32 GetSwapchainImageViewCount() const { return static_cast<uint32>(m_SwapchainImageViews.size()); }
    uint32 GetMinSwapchainImageCount() const { return m_MinSwapchainImageCount; }

    uint32 GetCurrentLocalFrame() const { return m_CurrentLocalFrame; }
    uint32 GetCurrentImageIndex() const { return m_CurrentImageIndex; }

    VkSemaphore GetImageAvailableSemaphore(uint32 Index) const { return m_ImageAvailabeSemaphores[Index]; }
    VkFence GetInflightFence(uint32 Index) const { return m_InflightFences[Index]; }

    VkSampleCountFlagBits GetCurrentMsaaSamples() const {return m_CurrentMsaaSamples; }

    CommandQueueVk* GetCommandQueue(CommandQueueType QueueType) const;

    VkSemaphore m_ImageAvailabeSemaphores[MAX_SWAPCHAIN_IMAGE_COUNT];
    VkSemaphore m_RenderFinishedSemaphores[MAX_SWAPCHAIN_IMAGE_COUNT];
    VkFence m_InflightFences[MAX_SWAPCHAIN_IMAGE_COUNT];
    VkFence m_ImagesInFlight[MAX_SWAPCHAIN_IMAGE_COUNT];

protected:
    void CheckCommandLists();

    C_STATUS CreateSurface(IWindow* Window);
    void DestroySurface();

    C_STATUS CreateSwapchain();
    C_STATUS CreateSwapchainImageViews();
    void CleanupSwapchain();

    C_STATUS CreateSyncObjects();
    void DestroySyncObjects();

    C_STATUS RecreateSwapchain();

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);

protected:
    RenderBackendVulkan* m_Backend = nullptr;
    IWindow* m_Window = nullptr;

    DeviceHandle m_Device{};
    VkPhysicalDevice m_PhysDeviceHandleCache;
    VkDevice m_DeviceHandleCache;

    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    
    uint32 m_MinSwapchainImageCount = 0;
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;
    VkFormat m_SwapchainImageFormat;
    VkExtent2D m_SwapchainExtent;

    bool m_FramebufferResized = false;

//     VkSemaphore m_ImageAvailabeSemaphores[MAX_FRAMES_IN_FLIGHT];
//     VkSemaphore m_RenderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
//     VkFence m_InflightFences[MAX_FRAMES_IN_FLIGHT];
//     VkFence m_ImagesInFlight[MAX_FRAMES_IN_FLIGHT];

    uint32 m_CurrentLocalFrame = 0;
    uint32 m_CurrentImageIndex = 0;

    VkSampleCountFlagBits m_CurrentMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
};

} // namespace Cyclone::Render
