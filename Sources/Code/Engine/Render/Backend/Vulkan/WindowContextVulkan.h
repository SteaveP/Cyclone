#pragma once

#include "CommonVulkan.h"
#include "Engine/Render/Types/WindowContext.h"

namespace Cyclone
{
class IWindow;
} // namespace Cyclone

namespace Cyclone::Render
{

class CCommandQueue;
class CommandQueueVulkan;

class WindowContextVulkan : public CWindowContext
{
protected:
    static const uint32 MAX_SWAPCHAIN_IMAGE_COUNT = 10;
public:
    WindowContextVulkan();
    virtual ~WindowContextVulkan();

    virtual C_STATUS Init(IRenderer* Renderer, IWindow* Window) override;
    virtual C_STATUS Shutdown() override;

    virtual C_STATUS BeginRender() override;
    virtual C_STATUS Present() override;

    virtual CCommandQueue* GetCommandQueue(CommandQueueType QueueType) const override;

    RenderBackendVulkan* GetBackend() const { return m_Backend; }
    IWindow* GetWindow() const { return m_Window; }

    DeviceHandle GetDevice() const { return m_Device; }
    VkPhysicalDevice GetPhysDevice() const { return m_PhysDeviceHandleCache; }
    VkDevice GetLogicDevice() const { return m_DeviceHandleCache; }

    VkFormat GetSwapchainImageFormat() const { return m_SwapchainImageFormat; }
    VkExtent2D GetSwapchainExtent() const { return m_SwapchainExtent; }
    VkImageView GetSwapchainImageView(uint32 Index) const { return m_SwapchainImageViews[Index]; }
    uint32 GetSwapchainImageViewCount() const { return static_cast<uint32>(m_SwapchainImageViews.size()); }
    uint32 GetMinSwapchainImageCount() const { return m_MinSwapchainImageCount; }

    VkSemaphore GetImageAvailableSemaphore(uint32 Index) const { return m_ImageAvailabeSemaphores[Index]; }
    VkFence GetInflightFence(uint32 Index) const { return m_InflightFences[Index]; }

    VkSampleCountFlagBits GetCurrentMsaaSamples() const {return m_CurrentMsaaSamples; }

    CommandQueueVulkan* GetCommandQueueVk(CommandQueueType QueueType) const;

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

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const Vector<VkSurfaceFormatKHR>& AvailableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const Vector<VkPresentModeKHR>& AvailablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);

protected:
    RenderBackendVulkan* m_Backend = nullptr;

    DeviceHandle m_Device{};
    VkPhysicalDevice m_PhysDeviceHandleCache;
    VkDevice m_DeviceHandleCache;

    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    
    uint32 m_MinSwapchainImageCount = 0;
    Vector<VkImage> m_SwapchainImages;
    Vector<VkImageView> m_SwapchainImageViews;
    VkFormat m_SwapchainImageFormat;
    VkExtent2D m_SwapchainExtent;

    bool m_FramebufferResized = false;

    VkSemaphore m_ImageAvailabeSemaphores[MAX_SWAPCHAIN_IMAGE_COUNT]{};
    VkSemaphore m_RenderFinishedSemaphores[MAX_SWAPCHAIN_IMAGE_COUNT]{};
    VkFence m_InflightFences[MAX_SWAPCHAIN_IMAGE_COUNT]{};
    VkFence m_ImagesInFlight[MAX_SWAPCHAIN_IMAGE_COUNT]{};

    VkSampleCountFlagBits m_CurrentMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
};

} // namespace Cyclone::Render
