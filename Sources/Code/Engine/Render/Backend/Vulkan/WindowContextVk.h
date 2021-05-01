#pragma once

#include "Common/CommonVulkan.h"

namespace Cyclone
{
class IWindow;
} // namespace Cyclone

namespace Cyclone::Render
{

class RenderBackendVulkan;
class CommandQueueVk;

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;
    std::optional<uint32_t> AsyncComputeFamily; // optional

    bool isComplete();
};

class WindowContextVk
{
public:
    WindowContextVk();
    ~WindowContextVk();
    C_STATUS Init(RenderBackendVulkan* RenderBackend, IWindow* Window);
    C_STATUS Shutdown();

    C_STATUS BeginRender();
    C_STATUS Present(VkSemaphore RenderFinishedSemaphore);

    RenderBackendVulkan* GetBackend() const { return m_backend; }
    IWindow* GetWindow() const { return m_window; }

    VkPhysicalDevice GetPhysDevice() const { return m_physicalDevice; }
    VkDevice GetDevice() const { return m_device; }

    CommandQueueVk* GetCommandQueue(CommandQueueType QueueType) { return m_commandQueues[(uint32_t)QueueType].get(); }

protected:
    void CheckCommandLists();

    C_STATUS pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices WindowContextVk::findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    C_STATUS createLogicalDevice();
    void destroyLogicalDevice();

    C_STATUS createSurface(IWindow* window);
    void destroySurface();

    C_STATUS createSwapchain();
    C_STATUS createSwapchainImageViews();

    void cleanupSwapchain();
    C_STATUS recreateSwapchain();

    C_STATUS createSyncObjects();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkSampleCountFlagBits getMaxUsableMSAASampleCount();

protected:
    // #todo_vk_fixme
public:
    RenderBackendVulkan* m_backend = nullptr;
    IWindow* m_window = nullptr;

    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
        
    std::unique_ptr<CommandQueueVk> m_commandQueues[(uint32_t)CommandQueueType::Count];

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    uint32_t m_minSwapchainImageCount;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    VkFormat m_swapchainImageFormat;
    VkExtent2D m_swapchainExtent;

    bool m_framebufferResized = false;

    std::vector<VkSemaphore> m_imageAvailabeSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inflightFences;
    std::vector<VkFence> m_imagesInFlight;

    size_t m_currentFrame = 0;
    uint32_t m_currentImageIndex = 0;

    VkSampleCountFlagBits m_currentMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkSampleCountFlagBits m_maxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
};

} // namespace Cyclone::Render
