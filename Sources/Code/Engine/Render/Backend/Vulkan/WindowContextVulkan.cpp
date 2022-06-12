#include "WindowContextVulkan.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"
#include "RenderBackendVk.h"

#include "CommandQueueVk.h"
#include "CommandBufferVk.h"

#if PLATFORM_WIN64
    #include <vulkan/vulkan_win32.h>
#else
    #error unsupported platform
#endif

namespace Cyclone::Render
{

static std::vector<std::string> GVkPhysicalDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

WindowContextVulkan::WindowContextVulkan() = default;
WindowContextVulkan::~WindowContextVulkan()
{
    CASSERT(m_Surface == VK_NULL_HANDLE);
}

C_STATUS WindowContextVulkan::Init(RenderBackendVulkan* RenderBackend, IWindow* Window)
{
    m_Backend = RenderBackend;
    m_Window = Window;
    
    if (m_Backend == nullptr || m_Window == nullptr)
        return C_STATUS::C_STATUS_INVALID_ARG;

    
    C_STATUS Result = CreateSurface(m_Window);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result); 

    {
        DeviceCreationDesc Desc{};
        Desc.Surface = m_Surface;
        Desc.EnabledPhysicalDeviceExtensions = GVkPhysicalDeviceExtensions;
        Result = m_Backend->GetGlobalContext().GetOrCreateDevice(Desc, m_Device);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

        m_PhysDeviceHandleCache = m_Backend->GetGlobalContext().GetPhysicalDevice(m_Device).PhysicalDeviceHandle;
        m_DeviceHandleCache = m_Backend->GetGlobalContext().GetLogicalDevice(m_Device).LogicalDeviceHandle;
    }

    Result = CreateSwapchain();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = CreateSwapchainImageViews();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = CreateSyncObjects();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS WindowContextVulkan::Shutdown()
{
    DestroySyncObjects();
    CleanupSwapchain();
    DestroySurface();

    return C_STATUS::C_STATUS_OK;
}

C_STATUS WindowContextVulkan::CreateSurface(IWindow* Window)
{
    // #todo_win #todo_vk make this platform independent
#if PLATFORM_WIN64
    VkWin32SurfaceCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    CreateInfo.hwnd = (HWND)Window->GetPlatformWindowHandle();
    CreateInfo.hinstance = GetModuleHandle(nullptr);

    VkResult Result = vkCreateWin32SurfaceKHR(m_Backend->GetGlobalContext().GetInstance(), &CreateInfo, nullptr, &m_Surface);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
#else
#error unsupported platform
#endif

    return C_STATUS::C_STATUS_OK;
}

void WindowContextVulkan::DestroySurface()
{
    if (m_Surface)
        vkDestroySurfaceKHR(m_Backend->GetGlobalContext().GetInstance(), m_Surface, nullptr);

    m_Surface = nullptr;
}

void WindowContextVulkan::CleanupSwapchain()
{
    VkDevice Device = GetLogicDevice();

    for (auto ImageView : m_SwapchainImageViews)
    {
        if (ImageView)
            vkDestroyImageView(Device, ImageView, nullptr);
    }
    m_SwapchainImageViews.clear();

    if (m_Swapchain)
        vkDestroySwapchainKHR(Device, m_Swapchain, nullptr);

    m_Swapchain = VK_NULL_HANDLE;
}

C_STATUS WindowContextVulkan::RecreateSwapchain()
{
    // #todo_vk m_swapchainImages ?
    int Width = m_Window->GetWidth();
    int Height = m_Window->GetHeight();
    while (Width == 0 || Height == 0)
    {
        // #todo_vk
        //glfwGetFramebufferSize(m_window, &width, &height);
        //glfwWaitEvents();
        
        CASSERT(false);
    }

    vkDeviceWaitIdle(GetLogicDevice());

    CleanupSwapchain();

    CreateSwapchain();
    CreateSwapchainImageViews();

    return C_STATUS::C_STATUS_OK;
}

C_STATUS WindowContextVulkan::CreateSwapchainImageViews()
{
    m_SwapchainImageViews.resize(m_SwapchainImages.size());

    for (size_t i = 0; i < m_SwapchainImageViews.size(); ++i)
    {
        m_SwapchainImageViews[i] = m_Backend->CreateImageView(m_Device, m_SwapchainImages[i], 1, m_SwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    return C_STATUS::C_STATUS_OK;
}

VkSurfaceFormatKHR WindowContextVulkan::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats)
{
    for (const auto& AvailableFormat : AvailableFormats)
    {
        if (AvailableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && AvailableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return AvailableFormat;
        }
    }

    return AvailableFormats[0];
}

VkPresentModeKHR WindowContextVulkan::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes)
{
    for (const auto& AvailablePresentMode : AvailablePresentModes) {
        if (AvailablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return AvailablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D WindowContextVulkan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Ccapabilities)
{
    if (Ccapabilities.currentExtent.width != UINT32_MAX)
    {
        return Ccapabilities.currentExtent;
    }
    else
    {
        int Width = m_Window->GetWidth();
        int Height = m_Window->GetHeight();
        VkExtent2D ActualExtent{ static_cast<uint32_t>(Width), static_cast<uint32_t>(Height) };

        ActualExtent.width = C_MAX(Ccapabilities.minImageExtent.width, C_MIN(Ccapabilities.maxImageExtent.width, ActualExtent.width));
        ActualExtent.height = C_MAX(Ccapabilities.minImageExtent.height, C_MIN(Ccapabilities.maxImageExtent.height, ActualExtent.height));

        return ActualExtent;
    }
}

C_STATUS WindowContextVulkan::CreateSwapchain()
{
    const PhysicalDevice& Device = m_Backend->GetGlobalContext().GetPhysicalDevice(m_Device);
    const LogicalDevice& LogicDevice = m_Backend->GetGlobalContext().GetLogicalDevice(m_Device);

    SwapChainSupportDetails SwapChainSupport =
        m_Backend->GetGlobalContext().QuerySwapChainSupport(Device.PhysicalDeviceHandle, m_Surface);

    VkSurfaceFormatKHR SurfaceFormat = ChooseSwapSurfaceFormat(SwapChainSupport.Formats);
    VkPresentModeKHR PresentMode = ChooseSwapPresentMode(SwapChainSupport.PresentModes);
    VkExtent2D Extent = ChooseSwapExtent(SwapChainSupport.Capabilities);

    m_MinSwapchainImageCount = SwapChainSupport.Capabilities.minImageCount;
    uint32_t ImagesCount = SwapChainSupport.Capabilities.minImageCount + 1;

    if (SwapChainSupport.Capabilities.maxImageCount > 0 && ImagesCount > SwapChainSupport.Capabilities.maxImageCount)
    {
        ImagesCount = SwapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    CreateInfo.surface = m_Surface;
    CreateInfo.minImageCount = ImagesCount;
    CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    CreateInfo.imageFormat = SurfaceFormat.format;
    CreateInfo.imageExtent = Extent;
    CreateInfo.imageArrayLayers = 1;
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices Indices =
        m_Backend->GetGlobalContext().FindQueueFamilies(Device.PhysicalDeviceHandle, m_Surface);

    uint32_t QueueFamilyIndices[] = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

    if (Indices.GraphicsFamily != Indices.PresentFamily)
    {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        CreateInfo.queueFamilyIndexCount = 2;
        CreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
    }
    else
    {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        CreateInfo.queueFamilyIndexCount = 0; // optional
        CreateInfo.pQueueFamilyIndices = nullptr; // optional
    }

    CreateInfo.preTransform = SwapChainSupport.Capabilities.currentTransform;
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    CreateInfo.presentMode = PresentMode;
    CreateInfo.clipped = VK_TRUE;
    CreateInfo.oldSwapchain = VK_NULL_HANDLE; // #todo_swapchain handle resize

    VkResult Result = vkCreateSwapchainKHR(LogicDevice.LogicalDeviceHandle, &CreateInfo, nullptr, &m_Swapchain);
    C_ASSERT_VK_SUCCEEDED(Result);

    // acquire swap chain images
    uint32_t SwapchainImagesCount;
    vkGetSwapchainImagesKHR(LogicDevice.LogicalDeviceHandle, m_Swapchain, &SwapchainImagesCount, nullptr);
    m_SwapchainImages.resize(SwapchainImagesCount);
    vkGetSwapchainImagesKHR(LogicDevice.LogicalDeviceHandle, m_Swapchain, &SwapchainImagesCount, m_SwapchainImages.data());

    m_SwapchainImageFormat = SurfaceFormat.format;
    m_SwapchainExtent = Extent;

    return C_STATUS::C_STATUS_OK;
}

C_STATUS WindowContextVulkan::CreateSyncObjects()
{
    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo FenceInfo{};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult Result;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        Result = vkCreateSemaphore(GetLogicDevice(), &SemaphoreInfo, nullptr, &m_ImageAvailabeSemaphores[i]);
        C_ASSERT_VK_SUCCEEDED(Result);

        Result = vkCreateSemaphore(GetLogicDevice(), &SemaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]);
        C_ASSERT_VK_SUCCEEDED(Result);

        Result = vkCreateFence(GetLogicDevice(), &FenceInfo, nullptr, &m_InflightFences[i]);
        C_ASSERT_VK_SUCCEEDED(Result);
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS WindowContextVulkan::BeginRender()
{
    vkWaitForFences(GetLogicDevice(), 1, &m_InflightFences[m_CurrentLocalFrame], VK_TRUE, UINT64_MAX);

    CheckCommandLists();

    VkResult result = vkAcquireNextImageKHR(GetLogicDevice(), m_Swapchain, UINT64_MAX, m_ImageAvailabeSemaphores[m_CurrentLocalFrame], VK_NULL_HANDLE, &m_CurrentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
        return C_STATUS::C_STATUS_OK;
    }
    else
    {
        CASSERT(result == VkResult::VK_SUCCESS || result == VkResult::VK_SUBOPTIMAL_KHR);
    }

    if (m_ImagesInFlight[m_CurrentImageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(GetLogicDevice(), 1, &m_ImagesInFlight[m_CurrentImageIndex], VK_TRUE, UINT64_MAX);
    }

    m_ImagesInFlight[m_CurrentImageIndex] = m_InflightFences[m_CurrentLocalFrame];

    return C_STATUS::C_STATUS_OK;
}

C_STATUS WindowContextVulkan::Present(VkSemaphore RenderFinishedSemaphore)
{
    // #todo_vk
    std::array<VkSemaphore, 20> WaitSemaphores;
    uint32_t WaitSemaphoresCount = 0;

    const auto& Device = m_Backend->GetGlobalContext().GetLogicalDevice(m_Device);

    auto* GraphicsQueue = Device.GetCommandQueue(CommandQueueType::Graphics);
    CASSERT(GraphicsQueue);

    for (uint32_t i = 0; i < GraphicsQueue->m_submittedCommandBuffers.size(); ++i)
    {
        if (GraphicsQueue->m_submittedCommandBuffers[i]->GetCompletedSemaphore() != VK_NULL_HANDLE)
        {
            WaitSemaphores[WaitSemaphoresCount] = GraphicsQueue->m_submittedCommandBuffers[i]->GetCompletedSemaphore();
            WaitSemaphoresCount++;
        }
    }

    if (RenderFinishedSemaphore != VK_NULL_HANDLE)
    {
        WaitSemaphores[WaitSemaphoresCount] = RenderFinishedSemaphore;
        WaitSemaphoresCount++;
    }

    VkPresentInfoKHR PresentInfo{};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = WaitSemaphoresCount;
    PresentInfo.pWaitSemaphores = WaitSemaphores.data();

    VkSwapchainKHR Swapchains[] = { m_Swapchain };
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = Swapchains;
    PresentInfo.pImageIndices = &m_CurrentImageIndex;
    PresentInfo.pResults = nullptr; // optional

    auto* PresentQueue = Device.GetCommandQueue(CommandQueueType::Present);
    CASSERT(PresentQueue);

    VkResult Result = vkQueuePresentKHR(PresentQueue->Get(), &PresentInfo);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
    {
        m_FramebufferResized = false;
        RecreateSwapchain();
    }
    else
    {
        C_ASSERT_VK_SUCCEEDED(Result);
    }

    m_CurrentLocalFrame = (m_CurrentLocalFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return C_STATUS::C_STATUS_OK;
}

void WindowContextVulkan::CheckCommandLists()
{
    const auto& Device = m_Backend->GetGlobalContext().GetLogicalDevice(m_Device);

    Device.GetCommandQueue(CommandQueueType::Graphics)->OnBeginFrame();
}

void WindowContextVulkan::DestroySyncObjects()
{
    VkDevice Device = GetLogicDevice();
    for (size_t i = 0; i < MAX_SWAPCHAIN_IMAGE_COUNT; ++i)
    {
        if (m_ImageAvailabeSemaphores[i])
            vkDestroySemaphore(Device, m_ImageAvailabeSemaphores[i], nullptr);
        m_ImageAvailabeSemaphores[i] = nullptr;

        if (m_RenderFinishedSemaphores[i])
            vkDestroySemaphore(Device, m_RenderFinishedSemaphores[i], nullptr);
        m_RenderFinishedSemaphores[i] = nullptr;

        if (m_InflightFences[i])
            vkDestroyFence(Device, m_InflightFences[i], nullptr);
        m_InflightFences[i] = nullptr;
        m_ImagesInFlight[i] = nullptr;
    }
}

CommandQueueVk* WindowContextVulkan::GetCommandQueue(CommandQueueType QueueType) const
{
    return m_Backend->GetGlobalContext().GetLogicalDevice(m_Device).GetCommandQueue(QueueType);
}

} // namespace Cyclone::Render
