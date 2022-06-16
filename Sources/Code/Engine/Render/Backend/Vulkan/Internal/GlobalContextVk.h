#pragma once

#include "CommonVulkan.h"
#include "Internal/ResourceManagerVk.h"

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
    Optional<uint32> GraphicsFamily;
    Optional<uint32> PresentFamily;
    Optional<uint32> AsyncComputeFamily; // optional

    bool IsComplete();
};

struct LogicalDevice
{
    VkDevice LogicalDeviceHandle = VK_NULL_HANDLE;
    VmaAllocator Allocator = VK_NULL_HANDLE;
    UniquePtr<CResourceManagerVk> ResourceManager;

    Array<UniquePtr<CommandQueueVulkan>, uint32(CommandQueueType::Count)> CommandQueues;

#if ENABLE_DEBUG_RENDER_BACKEND
    PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT;
#endif

    CommandQueueVulkan* GetCommandQueue(CommandQueueType QueueType) const { return CommandQueues[uint32(QueueType)].get(); }
};

struct PhysicalDevice
{
    VkPhysicalDevice PhysicalDeviceHandle = VK_NULL_HANDLE;
    VkSampleCountFlagBits MaxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;

    String Name;
    VkPhysicalDeviceType DeviceType;

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

    C_STATUS GetOrCreateDevice(DeviceCreationDesc Desc, CDeviceHandle& OutHandle);
    const PhysicalDevice& GetPhysicalDevice(CDeviceHandle Handle) const { return m_Devices[Handle.PhysicalDeviceHandle]; }
    const LogicalDevice& GetLogicalDevice(CDeviceHandle Handle) const { return GetPhysicalDevice(Handle).LogicalDevices[Handle.LogicalDeviceHandle]; }

    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice Device, VkSurfaceKHR Surface);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device, VkSurfaceKHR Surface);

protected:
    C_STATUS CreateInstance(InstanceCreationDesc Desc);
    void DestroyInstance();

    bool IsPhysicalDeviceSuitable(VkPhysicalDevice Device, VkSurfaceKHR Surface, Vector<String> PhysicalDeviceExtensions);
    C_STATUS PickPhysicalDevice(DeviceCreationDesc Desc, CDeviceHandle& OutHandle);

    bool IsLogicalDeviceSuitable(VkPhysicalDevice PhysDevice, VkDevice Device, VkSurfaceKHR Surface);
    C_STATUS CreateLogicalDevice(PhysicalDevice& PhysDevice, DeviceCreationDesc Desc, CDeviceHandle& OutHandle);
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
