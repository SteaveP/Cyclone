#include "GlobalContextVulkan.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"

#include "RenderBackendVulkan.h"

#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"

#include <iostream>

namespace Cyclone::Render
{

#ifdef _DEBUG
    const bool GVkEnableValidationLayers = true;
#else
    const bool GVkEnableValidationLayers = false;
#endif

// #todo_win #todo_vk make platform independent code without defines and make it extensible
Vector<const char*> GVkInstanceValidationLayers = { "VK_LAYER_KHRONOS_validation" };
Vector<const char*> GVkInstanceExtensions = {
#if PLATFORM_WIN64
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
#error unsupported platform
#endif
};

GlobalContextVulkan::GlobalContextVulkan() = default;

GlobalContextVulkan::~GlobalContextVulkan()
{
    CASSERT(m_Instance == VK_NULL_HANDLE);
    CASSERT(m_Devices.size() == 0);
}

C_STATUS GlobalContextVulkan::Init(RenderBackendVulkan* RenderBackend, InstanceCreationDesc Desc)
{
    m_Backend = RenderBackend;

    C_STATUS Result = CreateInstance(Desc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS GlobalContextVulkan::Shutdown()
{
    DestroyDevices();
    DestroyInstance();

    return C_STATUS::C_STATUS_OK;
}

C_STATUS GlobalContextVulkan::CreateInstance(InstanceCreationDesc Desc)
{
    if (GVkEnableValidationLayers && !CheckValidationlayerSupport())
    {
        CASSERT(false);
        return C_STATUS::C_STATUS_INVALID_ARG;
    }

    VkApplicationInfo AppInfo{};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = "CycloneApp";
    AppInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    AppInfo.pEngineName = "Cyclone";
    AppInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;

    // Instance Extensions
    Vector<const char*> InstanceExtensions(GVkInstanceExtensions.size() + Desc.EnabledExtensions.size());

    {
        uint32 count = 0;
        for (const auto& Name : GVkInstanceExtensions)
        {
            InstanceExtensions[count++] = Name;
        }
        for (const auto& Name : Desc.EnabledExtensions)
        {
            InstanceExtensions[count++] = Name.c_str();
        }

        CreateInfo.ppEnabledExtensionNames = InstanceExtensions.data();
        CreateInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());
    }

    // Instance Layers
    Vector<const char*> InstanceLayers(Desc.EnabledLayers.size());
    {
        uint32 count = 0;
        for (const auto& Name : Desc.EnabledLayers)
        {
            InstanceLayers[count++] = Name.c_str();
        }

        if (GVkEnableValidationLayers)
        {
            for (const auto& Name : GVkInstanceValidationLayers)
            {
                InstanceLayers.push_back(Name);
                count++;
            }
        }

        CreateInfo.ppEnabledLayerNames = InstanceLayers.data();
        CreateInfo.enabledLayerCount = static_cast<uint32_t>(InstanceLayers.size());
    }

    VkResult Result = vkCreateInstance(&CreateInfo, nullptr, &m_Instance);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    if (1) // #todo_vk
    {
        uint32_t ExtensionsCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionsCount, nullptr);

        Vector<VkExtensionProperties> Extensions(ExtensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionsCount, Extensions.data());

        std::cout << "Vulkan available extensions:\n";
        for (const auto& Extension : Extensions)
        {
            std::cout << '\t' << Extension.extensionName << '\n';
        }
    }

    return C_STATUS::C_STATUS_OK;
}

void GlobalContextVulkan::DestroyInstance()
{
    if (m_Instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }
}

bool GlobalContextVulkan::CheckValidationlayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    Vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto& layerName : GVkInstanceValidationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

C_STATUS GlobalContextVulkan::GetOrCreateDevice(DeviceCreationDesc Desc, DeviceHandle& OutHandle)
{
    OutHandle = DeviceHandle{};

    // Check cache for physical device first
    for (uint32 i = 0; i < m_Devices.size(); ++i)
    {
        const auto& PhysDevice = m_Devices[i];
        if (IsPhysicalDeviceSuitable(PhysDevice.PhysicalDeviceHandle, Desc.Surface, Desc.EnabledPhysicalDeviceExtensions))
        {
            OutHandle.PhysicalDeviceHandle = i;
        }
    }

    // Create physical device otherwise
    C_STATUS Result = C_STATUS::C_STATUS_OK;

    if (!OutHandle.IsPhysicalDeviceHandleValid())
    {
        Result = PickPhysicalDevice(Desc, OutHandle);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    PhysicalDevice& PhysDevice = m_Devices[OutHandle.PhysicalDeviceHandle];

    // Check cache for logical device
    for (uint32 j = 0; j < PhysDevice.LogicalDevices.size(); ++j)
    {
        const auto& LogicalDevice = PhysDevice.LogicalDevices[j];
        if (IsLogicalDeviceSuitable(PhysDevice.PhysicalDeviceHandle, LogicalDevice.LogicalDeviceHandle, Desc.Surface))
        {
            OutHandle.LogicalDeviceHandle = j;

            C_ASSERT_RETURN_VAL(OutHandle.IsPhysicalDeviceHandleValid() && OutHandle.IsLogicalDeviceHandleValid(), C_STATUS::C_STATUS_ERROR);
            return C_STATUS::C_STATUS_OK;
        }
    }

    // Create otherwise
    {
        Result = CreateLogicalDevice(PhysDevice, Desc, OutHandle);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    C_ASSERT_RETURN_VAL(OutHandle.IsPhysicalDeviceHandleValid() && OutHandle.IsLogicalDeviceHandleValid(), C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

Cyclone::C_STATUS GlobalContextVulkan::PickPhysicalDevice(DeviceCreationDesc Desc, DeviceHandle& OutHandle)
{
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(GetInstance(), &DeviceCount, nullptr);

    C_ASSERT_RETURN_VAL(DeviceCount != 0, C_STATUS::C_STATUS_ERROR);

    Vector<VkPhysicalDevice> Devices(DeviceCount);
    vkEnumeratePhysicalDevices(GetInstance(), &DeviceCount, Devices.data());

    for (const auto& Device : Devices)
    {
        if (IsPhysicalDeviceSuitable(Device, Desc.Surface, Desc.EnabledPhysicalDeviceExtensions))
        {
            OutHandle.PhysicalDeviceHandle = static_cast<uint16>(m_Devices.size());
            auto& PhysDevice = m_Devices.emplace_back();

            PhysDevice.PhysicalDeviceHandle = Device;
            PhysDevice.MaxMsaaSamples = GetMaxUsableMSAASampleCount(Device);
            
            return C_STATUS::C_STATUS_OK;
        }
    }

    return C_STATUS::C_STATUS_ERROR;
}

//////////////////////////////////////////////////////////////////////////


C_STATUS GlobalContextVulkan::CreateLogicalDevice(PhysicalDevice& PhysDevice, DeviceCreationDesc Desc, DeviceHandle& OutHandle)
{
    QueueFamilyIndices Indices = FindQueueFamilies(PhysDevice.PhysicalDeviceHandle, Desc.Surface);

    Vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
    std::set<uint32_t> UniqueQueueIndices = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

    if (Indices.AsyncComputeFamily.has_value())
        UniqueQueueIndices.insert(Indices.AsyncComputeFamily.value());

    float QueuePriority = 1.f;
    for (uint32_t QueueFamily : UniqueQueueIndices)
    {
        VkDeviceQueueCreateInfo QueueCreateInfo{};
        QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfo.queueFamilyIndex = QueueFamily;
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

    Vector<const char*> DeviceExtensions(Desc.EnabledPhysicalDeviceExtensions.size());
    {
        for (uint32 i = 0; i < DeviceExtensions.size(); ++i)
        {
            DeviceExtensions[i] = Desc.EnabledPhysicalDeviceExtensions[i].c_str();
        }
        CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();
        CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    }

    OutHandle.LogicalDeviceHandle = static_cast<uint16>(PhysDevice.LogicalDevices.size());
    LogicalDevice& LogicDevice = PhysDevice.LogicalDevices.emplace_back();

    VkResult Result = vkCreateDevice(PhysDevice.PhysicalDeviceHandle, &CreateInfo, nullptr, &LogicDevice.LogicalDeviceHandle);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    // #todo_vk refactor #todo_move
    auto InitQueue = [&](CommandQueueType QueueType, uint32_t QueueFamilyIndex, uint32_t QueueIndex)
    {
        UniquePtr<CommandQueueVulkan> CommandQueue(static_cast<CommandQueueVulkan*>(m_Backend->CreateCommandQueue()));
        C_STATUS res = CommandQueue->Init(m_Backend, OutHandle, QueueType, QueueFamilyIndex, QueueIndex);
        CASSERT(C_SUCCEEDED(res));

        LogicDevice.CommandQueues[(uint32_t)QueueType] = std::move(CommandQueue);
    };

    if (Indices.GraphicsFamily.has_value())
        InitQueue(CommandQueueType::Graphics, Indices.GraphicsFamily.value(), 0);
    if (Indices.PresentFamily.has_value())
        InitQueue(CommandQueueType::Present, Indices.PresentFamily.value(), 0);
    if (Indices.AsyncComputeFamily.has_value())
        InitQueue(CommandQueueType::AsyncCompute, Indices.AsyncComputeFamily.value(), 0);

    return C_STATUS::C_STATUS_OK;
}

bool GlobalContextVulkan::IsPhysicalDeviceSuitable(VkPhysicalDevice Device, VkSurfaceKHR Surface, Vector<std::string> PhysicalDeviceExtensions)
{
#if 0
    VkPhysicalDeviceProperties DeviceProperties;
    vkGetPhysicalDeviceProperties(Device, &DeviceProperties);

    VkPhysicalDeviceFeatures DeviceFeatures;
    vkGetPhysicalDeviceFeatures(Device, &DeviceFeatures);

    return DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        && DeviceFeatures.geometryShader;
#else

    bool IsExtensionsSupported = CheckDeviceExtensionSupport(Device, PhysicalDeviceExtensions);

    VkPhysicalDeviceFeatures DeviceFeatures;
    vkGetPhysicalDeviceFeatures(Device, &DeviceFeatures);

    bool IsSwapChainAdequate = false;
    if (IsExtensionsSupported)
    {
        SwapChainSupportDetails SwapChainSupport = QuerySwapChainSupport(Device, Surface);
        IsSwapChainAdequate = !SwapChainSupport.Formats.empty() && !SwapChainSupport.PresentModes.empty();
    }

    QueueFamilyIndices Indices = FindQueueFamilies(Device, Surface);
    return Indices.IsComplete() && IsExtensionsSupported && IsSwapChainAdequate && DeviceFeatures.samplerAnisotropy;
#endif
}

bool QueueFamilyIndices::IsComplete()
{
    return GraphicsFamily.has_value() && PresentFamily.has_value() && AsyncComputeFamily.has_value();
}

QueueFamilyIndices GlobalContextVulkan::FindQueueFamilies(VkPhysicalDevice Device, VkSurfaceKHR Surface)
{
    QueueFamilyIndices Indices{};

    uint32_t QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);

    Vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilies.data());

    int i = 0;
    for (const auto& QueueFamily : QueueFamilies)
    {
        VkBool32 PresentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, Surface, &PresentSupport);

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

        if (Indices.IsComplete())
            break;

        i++;
    }

    return Indices;
}

VkSampleCountFlagBits GlobalContextVulkan::GetMaxUsableMSAASampleCount(VkPhysicalDevice Device)
{
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(Device, &physicalDeviceProperties);

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

SwapChainSupportDetails GlobalContextVulkan::QuerySwapChainSupport(VkPhysicalDevice Device, VkSurfaceKHR Surface)
{
    SwapChainSupportDetails Details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface, &Details.Capabilities);

    uint32_t FormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, nullptr);

    if (FormatCount != 0)
    {
        Details.Formats.resize(FormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, Details.Formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        Details.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &presentModeCount, Details.PresentModes.data());
    }

    return Details;
}

bool GlobalContextVulkan::CheckDeviceExtensionSupport(VkPhysicalDevice device, Vector<std::string> PhysicalDeviceExtensions)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    Vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(PhysicalDeviceExtensions.begin(), PhysicalDeviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool GlobalContextVulkan::IsLogicalDeviceSuitable(VkPhysicalDevice PhysDevice, VkDevice Device, VkSurfaceKHR Surface)
{
    QueueFamilyIndices Indices = FindQueueFamilies(PhysDevice, Surface);
    return Indices.GraphicsFamily.has_value() && Indices.PresentFamily.has_value();

}

void GlobalContextVulkan::DestroyDevices()
{
    // #todo_vk refactor to destructors
    for (auto& PhysDevice : m_Devices)
    {
        for (auto& LogicDevice : PhysDevice.LogicalDevices)
        {
            for (auto& Queue : LogicDevice.CommandQueues)
            {
                Queue.reset();
            }
            vkDestroyDevice(LogicDevice.LogicalDeviceHandle, nullptr);
        }

        PhysDevice.LogicalDevices.clear();
    }

    m_Devices.clear();
}

} // namespace Cyclone::Render
