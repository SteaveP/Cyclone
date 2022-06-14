#pragma once

#include "CommonVulkan.h"

namespace Cyclone::Render
{

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR Capabilities;
    Vector<VkSurfaceFormatKHR> Formats;
    Vector<VkPresentModeKHR> PresentModes;
};

struct QueueFamilyIndices
{
    std::optional<uint32> GraphicsFamily;
    std::optional<uint32> PresentFamily;
    std::optional<uint32> AsyncComputeFamily; // optional

    bool IsComplete();
};

struct LogicalDevice
{
    VkDevice LogicalDeviceHandle = VK_NULL_HANDLE;

    Array<UniquePtr<CommandQueueVulkan>, uint32(CommandQueueType::Count)> CommandQueues;

    CommandQueueVulkan* GetCommandQueue(CommandQueueType QueueType) const { return CommandQueues[uint32(QueueType)].get(); }
};

struct PhysicalDevice
{
    VkPhysicalDevice PhysicalDeviceHandle = VK_NULL_HANDLE;
    VkSampleCountFlagBits MaxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;

    Vector<LogicalDevice> LogicalDevices;
};

struct InstanceCreationDesc
{
    Vector<String> EnabledLayers;
    Vector<String> EnabledExtensions;

};
struct DeviceCreationDesc
{
    VkSurfaceKHR Surface = VK_NULL_HANDLE;
    Vector<String> EnabledPhysicalDeviceExtensions;
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

    bool IsPhysicalDeviceSuitable(VkPhysicalDevice Device, VkSurfaceKHR Surface, Vector<String> PhysicalDeviceExtensions);
    C_STATUS PickPhysicalDevice(DeviceCreationDesc Desc, DeviceHandle& OutHandle);

    bool IsLogicalDeviceSuitable(VkPhysicalDevice PhysDevice, VkDevice Device, VkSurfaceKHR Surface);
    C_STATUS CreateLogicalDevice(PhysicalDevice& PhysDevice, DeviceCreationDesc Desc, DeviceHandle& OutHandle);
    void DestroyDevices();
    
    bool CheckDeviceExtensionSupport(VkPhysicalDevice Device, Vector<String> PhysicalDeviceExtensions);
    bool CheckValidationlayerSupport();

    VkSampleCountFlagBits GetMaxUsableMSAASampleCount(VkPhysicalDevice Device);

protected:
    RenderBackendVulkan* m_Backend = nullptr;

    VkInstance m_Instance = VK_NULL_HANDLE;

    Vector<PhysicalDevice> m_Devices;
};

} // namespace Cyclone::Render
