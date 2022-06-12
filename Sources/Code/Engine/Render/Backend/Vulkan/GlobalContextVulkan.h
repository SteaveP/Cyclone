#pragma once

#include "Common/CommonVulkan.h"


namespace Cyclone::Render
{

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

    bool IsComplete();
};

struct LogicalDevice
{
    VkDevice LogicalDeviceHandle = VK_NULL_HANDLE;

    std::unique_ptr<CommandQueueVk> CommandQueues[(uint32_t)CommandQueueType::Count]{};

    CommandQueueVk* GetCommandQueue(CommandQueueType QueueType) const { return CommandQueues[(uint32_t)QueueType].get(); }
};

struct PhysicalDevice
{
    VkPhysicalDevice PhysicalDeviceHandle = VK_NULL_HANDLE;
    VkSampleCountFlagBits MaxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<LogicalDevice> LogicalDevices;
};

struct InstanceCreationDesc
{
    std::vector<std::string> EnabledLayers;
    std::vector<std::string> EnabledExtensions;

};
struct DeviceCreationDesc
{
    VkSurfaceKHR Surface = VK_NULL_HANDLE;
    std::vector<std::string> EnabledPhysicalDeviceExtensions; //= { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

class GlobalContextVulkan
{
public:
    GlobalContextVulkan();
    ~GlobalContextVulkan();

    C_STATUS Init(RenderBackendVulkan* RenderBackend, InstanceCreationDesc Desc);
    C_STATUS Shutdown();

    RenderBackendVulkan* GetBackend() const { return m_Backend; }

    VkInstance GetInstance() const { return m_Instance; }

    C_STATUS GetOrCreateDevice(DeviceCreationDesc Desc, DeviceHandle& OutHandle);
    const PhysicalDevice& GetPhysicalDevice(DeviceHandle Handle) const { return m_Devices[Handle.PhysicalDeviceHandle]; }
    const LogicalDevice& GetLogicalDevice(DeviceHandle Handle) const { return GetPhysicalDevice(Handle).LogicalDevices[Handle.LogicalDeviceHandle]; }

    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice Device, VkSurfaceKHR Surface);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device, VkSurfaceKHR Surface);

protected:
    C_STATUS CreateInstance(InstanceCreationDesc Desc);
    void DestroyInstance();

    bool IsPhysicalDeviceSuitable(VkPhysicalDevice Device, VkSurfaceKHR Surface, std::vector<std::string> PhysicalDeviceExtensions);
    C_STATUS PickPhysicalDevice(DeviceCreationDesc Desc, DeviceHandle& OutHandle);

    bool IsLogicalDeviceSuitable(VkPhysicalDevice PhysDevice, VkDevice Device, VkSurfaceKHR Surface);
    C_STATUS CreateLogicalDevice(PhysicalDevice& PhysDevice, DeviceCreationDesc Desc, DeviceHandle& OutHandle);
    void DestroyDevices();
    
    bool CheckDeviceExtensionSupport(VkPhysicalDevice Device, std::vector<std::string> PhysicalDeviceExtensions);
    bool CheckValidationlayerSupport();

    VkSampleCountFlagBits GetMaxUsableMSAASampleCount(VkPhysicalDevice Device);

protected:
    RenderBackendVulkan* m_Backend = nullptr;

    VkInstance m_Instance = VK_NULL_HANDLE;

    std::vector<PhysicalDevice> m_Devices;
};

} // namespace Cyclone::Render
