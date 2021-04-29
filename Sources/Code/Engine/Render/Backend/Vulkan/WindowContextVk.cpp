#include "WindowContextVk.h"

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

#ifdef _DEBUG
    const bool g_enableValidationLayers = true;
#else
    const bool g_enableValidationLayers = false;
#endif

static std::vector<const char*> g_validationLayers = { "VK_LAYER_KHRONOS_validation" };
static std::vector<const char*> g_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

C_STATUS WindowContextVk::Init(RenderBackendVulkan* RenderBackend, IWindow* Window)
{
    m_backend = RenderBackend;
    m_window = Window;
    
    if (m_window == nullptr)
        return C_STATUS::C_STATUS_OK;

    C_STATUS result = createSurface(m_window);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result); 

    result = pickPhysicalDevice();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);

    result = createLogicalDevice();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);

    result = createSwapchain();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);

    result = createSwapchainImageViews();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);

    result = createSyncObjects();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS WindowContextVk::Shutdown()
{
    cleanupSwapchain();
    destroySurface();
    destroyLogicalDevice();

    return C_STATUS::C_STATUS_OK;
}


C_STATUS WindowContextVk::createLogicalDevice()
{
    QueueFamilyIndices Indices = findQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
    std::set<uint32_t> UniqueQueueIndices = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

    if (Indices.AsyncComputeFamily.has_value())
        UniqueQueueIndices.insert(Indices.AsyncComputeFamily.value());

    float QueuePriority = 1.f;
    for (uint32_t queueFamily : UniqueQueueIndices)
    {
        VkDeviceQueueCreateInfo QueueCreateInfo{};
        QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfo.queueFamilyIndex = queueFamily;
        QueueCreateInfo.queueCount = 1;

        QueueCreateInfo.pQueuePriorities = &QueuePriority;

        QueueCreateInfos.emplace_back(std::move(QueueCreateInfo));
    }

    VkPhysicalDeviceFeatures PhysicalDeviceFeatures{};
    PhysicalDeviceFeatures.samplerAnisotropy = true;

    VkDeviceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
    CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
    CreateInfo.pEnabledFeatures = &PhysicalDeviceFeatures;

    // logical device extensions and layers are obsolete now, but for older implementations can be mandatory (added here for example)
    {
        CreateInfo.enabledExtensionCount = static_cast<uint32_t>(g_deviceExtensions.size());
        CreateInfo.ppEnabledExtensionNames = g_deviceExtensions.data();

        CreateInfo.enabledLayerCount = 0;

        if (g_enableValidationLayers) {
            CreateInfo.enabledLayerCount = static_cast<uint32_t>(g_validationLayers.size());
            CreateInfo.ppEnabledLayerNames = g_validationLayers.data();
        }
        else {
            CreateInfo.enabledLayerCount = 0;
        }
    }

    VkResult Result = vkCreateDevice(m_physicalDevice, &CreateInfo, nullptr, &m_device);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    auto InitQueue = [&](CommandQueueType QueueType, uint32_t QueueFamilyIndex, uint32_t QueueIndex)
    {
        auto CommandQueue = std::make_unique<CommandQueueVk>();
        C_STATUS res = CommandQueue->Init(m_backend, QueueType, QueueFamilyIndex, QueueIndex);
        CASSERT(C_SUCCEEDED(res));

        m_commandQueues[(uint32_t)QueueType] = std::move(CommandQueue);
    };

    if (Indices.GraphicsFamily.has_value())
        InitQueue(CommandQueueType::Graphics, Indices.GraphicsFamily.value(), 0);
    if (Indices.PresentFamily.has_value())
        InitQueue(CommandQueueType::Present, Indices.PresentFamily.value(), 0);
    if (Indices.AsyncComputeFamily.has_value())
        InitQueue(CommandQueueType::AsyncCompute, Indices.AsyncComputeFamily.value(), 0);

    return C_STATUS::C_STATUS_OK;
}

void WindowContextVk::destroyLogicalDevice()
{
    vkDestroyDevice(m_device, nullptr);
}

C_STATUS WindowContextVk::pickPhysicalDevice()
{
    const auto& globalContext = m_backend->GetGlobalContext();
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(globalContext.GetInstance(), &deviceCount, nullptr);

    C_ASSERT_RETURN_VAL(deviceCount != 0, C_STATUS::C_STATUS_ERROR);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(globalContext.GetInstance(), &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            m_physicalDevice = device;
            m_maxMsaaSamples = getMaxUsableMSAASampleCount();
            break;
        }
    }

    C_ASSERT_RETURN_VAL(m_physicalDevice != VK_NULL_HANDLE, C_STATUS::C_STATUS_ERROR);
    return C_STATUS::C_STATUS_OK;
}

bool WindowContextVk::isDeviceSuitable(VkPhysicalDevice device)
{
#if 0
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        && deviceFeatures.geometryShader;
#else
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;
#endif
}

VkSampleCountFlagBits WindowContextVk::getMaxUsableMSAASampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT)
        return VK_SAMPLE_COUNT_64_BIT;
    else if (counts & VK_SAMPLE_COUNT_32_BIT)
        return VK_SAMPLE_COUNT_32_BIT;
    else if (counts & VK_SAMPLE_COUNT_16_BIT)
        return VK_SAMPLE_COUNT_16_BIT;
    else if (counts & VK_SAMPLE_COUNT_8_BIT)
        return VK_SAMPLE_COUNT_8_BIT;
    else if (counts & VK_SAMPLE_COUNT_4_BIT)
        return VK_SAMPLE_COUNT_4_BIT;
    else if (counts & VK_SAMPLE_COUNT_2_BIT)
        return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}


bool WindowContextVk::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(g_deviceExtensions.begin(), g_deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices WindowContextVk::findQueueFamilies(VkPhysicalDevice Device)
{
    QueueFamilyIndices Indices{};

    uint32_t QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilies.data());

    int i = 0;
    for (const auto& QueueFamily : QueueFamilies)
    {
        VkBool32 PresentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, m_surface, &PresentSupport);

        // #todo_vk async compute also can have present capability, need to properly handle that
        // right now just pick first one with hope that it will be with the graphics bit
        if (!Indices.PresentFamily.has_value() && PresentSupport)
        {
            Indices.PresentFamily = i;
        }

        if (!Indices.GraphicsFamily.has_value() && QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            Indices.GraphicsFamily = i;
        }
        else if (!Indices.AsyncComputeFamily.has_value() && QueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            Indices.AsyncComputeFamily = i;
        }

        if (Indices.isComplete())
            break;

        i++;
    }

    return Indices;
}


C_STATUS WindowContextVk::createSurface(IWindow* window)
{
#if PLATFORM_WIN64
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = (HWND)window->GetPlatformWindowHandle();
    createInfo.hinstance = GetModuleHandle(nullptr);

    VkResult result = vkCreateWin32SurfaceKHR(m_backend->GetGlobalContext().GetInstance(), &createInfo, nullptr, &m_surface);
    C_ASSERT_VK_SUCCEEDED_RET(result, C_STATUS::C_STATUS_ERROR);
#else
#error unsupported platform
#endif

    return C_STATUS::C_STATUS_OK;
}

void WindowContextVk::destroySurface()
{
    vkDestroySurfaceKHR(m_backend->GetGlobalContext().GetInstance(), m_surface, nullptr);
}

void WindowContextVk::cleanupSwapchain()
{
    for (auto imageView : m_swapchainImageViews)
    {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

C_STATUS WindowContextVk::recreateSwapchain()
{
    // #todo_vk m_swapchainImages ?
    int width = m_window->GetWidth();
    int height = m_window->GetHeight();
    while (width == 0 || height == 0)
    {
        // #todo_vk
        //glfwGetFramebufferSize(m_window, &width, &height);
        //glfwWaitEvents();
        CASSERT(false);
    }

    vkDeviceWaitIdle(m_device);

    cleanupSwapchain();

    createSwapchain();
    createSwapchainImageViews();
    //createRenderPass();
    //createGraphicsPipeline();
    //createColorResources();
    //createDepthResources();
    //createFrameBuffers();
    //createUniformBuffers();
    //createDescriptorPool();
    //createDescriptorSets();
    //createCommandBuffers();

    return C_STATUS::C_STATUS_OK;
}

SwapChainSupportDetails WindowContextVk::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.Capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.Formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.Formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.PresentModes.data());
    }

    return details;
}
C_STATUS WindowContextVk::createSwapchainImageViews()
{
    m_swapchainImageViews.resize(m_swapchainImages.size());

    for (size_t i = 0; i < m_swapchainImageViews.size(); ++i)
    {
        m_swapchainImageViews[i] = m_backend->CreateImageView(m_swapchainImages[i], 1, m_swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    return C_STATUS::C_STATUS_OK;
}

VkSurfaceFormatKHR WindowContextVk::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR WindowContextVk::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D WindowContextVk::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width = m_window->GetWidth();
        int height = m_window->GetHeight();
        VkExtent2D actualExtent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        actualExtent.width = C_MAX(capabilities.minImageExtent.width, C_MIN(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = C_MAX(capabilities.minImageExtent.height, C_MIN(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

C_STATUS WindowContextVk::createSwapchain()
{
    SwapChainSupportDetails SwapChainSupport = querySwapChainSupport(m_physicalDevice);

    VkSurfaceFormatKHR SurfaceFormat = chooseSwapSurfaceFormat(SwapChainSupport.Formats);
    VkPresentModeKHR PresentMode = chooseSwapPresentMode(SwapChainSupport.PresentModes);
    VkExtent2D Extent = chooseSwapExtent(SwapChainSupport.Capabilities);

    uint32_t ImagesCount = SwapChainSupport.Capabilities.minImageCount + 1;

    if (SwapChainSupport.Capabilities.maxImageCount > 0 && ImagesCount > SwapChainSupport.Capabilities.maxImageCount)
    {
        ImagesCount = SwapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    CreateInfo.surface = m_surface;
    CreateInfo.minImageCount = ImagesCount;
    CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    CreateInfo.imageFormat = SurfaceFormat.format;
    CreateInfo.imageExtent = Extent;
    CreateInfo.imageArrayLayers = 1;
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices Indices = findQueueFamilies(m_physicalDevice);
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

    VkResult Result = vkCreateSwapchainKHR(m_device, &CreateInfo, nullptr, &m_swapchain);
    C_ASSERT_VK_SUCCEEDED(Result);

    // acquire swap chain images
    uint32_t SwapchainImagesCount;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &SwapchainImagesCount, nullptr);
    m_swapchainImages.resize(SwapchainImagesCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &SwapchainImagesCount, m_swapchainImages.data());

    m_swapchainImageFormat = SurfaceFormat.format;
    m_swapchainExtent = Extent;

    return C_STATUS::C_STATUS_OK;
}

bool QueueFamilyIndices::isComplete()
{
    return GraphicsFamily.has_value() && PresentFamily.has_value() && AsyncComputeFamily.has_value();
}

C_STATUS WindowContextVk::createSyncObjects()
{
    m_imageAvailabeSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_inflightFences.resize(MAX_FRAMES_IN_FLIGHT);
    m_imagesInFlight.resize(m_swapchainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo FenceInfo{};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult result;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        result = vkCreateSemaphore(m_device, &SemaphoreInfo, nullptr, &m_imageAvailabeSemaphores[i]);
        C_ASSERT_VK_SUCCEEDED(result);

        result = vkCreateSemaphore(m_device, &SemaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]);
        C_ASSERT_VK_SUCCEEDED(result);

        result = vkCreateFence(m_device, &FenceInfo, nullptr, &m_inflightFences[i]);
        C_ASSERT_VK_SUCCEEDED(result);
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS WindowContextVk::BeginRender()
{
    vkWaitForFences(m_device, 1, &m_inflightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    CheckCommandLists();

    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailabeSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return C_STATUS::C_STATUS_OK;
    }
    else
    {
        CASSERT(result == VkResult::VK_SUCCESS || result == VkResult::VK_SUBOPTIMAL_KHR);
    }

    if (m_imagesInFlight[m_currentImageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(m_device, 1, &m_imagesInFlight[m_currentImageIndex], VK_TRUE, UINT64_MAX);
    }

    m_imagesInFlight[m_currentImageIndex] = m_inflightFences[m_currentFrame];

    return C_STATUS::C_STATUS_OK;
}

C_STATUS WindowContextVk::Present(VkSemaphore RenderFinishedSemaphore)
{
    // #todo_vk
    std::array<VkSemaphore, 20> WaitSemaphores;
    uint32_t WaitSemaphoresCount = 0;

    auto* GraphicsQueue = GetCommandQueue(CommandQueueType::Graphics);
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

    VkSwapchainKHR Swapchains[] = { m_swapchain };
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = Swapchains;
    PresentInfo.pImageIndices = &m_currentImageIndex;
    PresentInfo.pResults = nullptr; // optional

    auto* PresentQueue = GetCommandQueue(CommandQueueType::Present);
    CASSERT(PresentQueue);

    VkResult Result = vkQueuePresentKHR(PresentQueue->GetQueue(), &PresentInfo);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
    {
        m_framebufferResized = false;
        recreateSwapchain();
    }
    else
    {
        C_ASSERT_VK_SUCCEEDED(Result);
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return C_STATUS::C_STATUS_OK;
}

void WindowContextVk::CheckCommandLists()
{
    GetCommandQueue(CommandQueueType::Graphics)->OnBeginFrame();
}

WindowContextVk::WindowContextVk() = default;
WindowContextVk::~WindowContextVk() = default;

} // namespace Cyclone::Render
