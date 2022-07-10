#pragma once

#include "DeviceFeatureInfoVulkan.h"
#include "ResourceManagerVulkan.h"
#include "DisposalManagerVulkan.h"

namespace Cyclone::Render
{

struct CInstanceCreationDesc
{
    Vector<String> EnabledLayers;
    Vector<String> EnabledExtensions;
};

struct CPhysicalDevice
{
    VkPhysicalDevice DeviceVk = VK_NULL_HANDLE;

    uint32 ID = 0;
    String Name;

    CPhysicalDeviceFeatures Features;
    CPhysicalDeviceRequestedFeatures RequestedFeatures;
};

struct CDeviceTable
{
#if ENABLE_DEBUG_RENDER_BACKEND
    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = nullptr;
    PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = nullptr;
    PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT = nullptr;

    PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXT = nullptr;
    PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXT = nullptr;
#endif
};

// #todo_vk expose to engine in a crossplatform way?
struct CDevice
{
public:
    VkDevice DeviceVk = VK_NULL_HANDLE;

#if ENABLE_VOLK_LOADER
    VolkDeviceTable DeviceTable{};
#endif
    CDeviceTable DeviceTableExt{};

    VmaAllocator Allocator = VK_NULL_HANDLE;
    UniquePtr<CResourceManagerVulkan> ResourceManager;
    UniquePtr<CBindlessDescriptorManagerVulkan> BindlessManager;
    UniquePtr<CUploadQueue> UploadQueue;
    UniquePtr<CDisposalManagerVulkan> DisposalManager;

    Array<UniquePtr<CCommandQueueVulkan>, uint32(CommandQueueType::Count)> CommandQueues;

    uint32 ReferenceCount = 0;

    CDeviceHandle DeviceHandle;
    CDeviceFeaturesInfo RequestedFeatures;

public:
    CCommandQueueVulkan* GetCommandQueue(CommandQueueType QueueType) const { return CommandQueues[uint32(QueueType)].get(); }
};

class CDeviceManagerVulkan
{
public:
    CDeviceManagerVulkan();
    ~CDeviceManagerVulkan();

    C_STATUS Init(CRenderBackendVulkan* RenderBackend, const CInstanceCreationDesc& Desc);
    C_STATUS Shutdown();

    C_STATUS GetOrCreateDevice(VkSurfaceKHR Surface, CDeviceHandle& OutHandle);
    C_STATUS ReleaseDevice(CDeviceHandle Handle);

    VkInstance GetInstanceVk() const { return m_InstanceVk; }
    CRenderBackendVulkan* GetBackendVk() const { return m_BackendVk; }

    const CPhysicalDevice& GetPhysDevice(CDeviceHandle Handle) const { return m_PhysDevices[Handle.PhysDeviceHandle]; }
    const CDevice& GetDevice(CDeviceHandle Handle) const { return m_Devices[Handle.DeviceHandle]; }

    uint32 GetPhysDeviceCount() const { return (uint32)m_PhysDevices.size(); }
    uint32 GetDeviceCount() const { return (uint32)m_Devices.size(); }

    CSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice Device, VkSurfaceKHR Surface) const;
    CQueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device, VkSurfaceKHR Surface) const;

protected:
    C_STATUS CreateInstance(const CInstanceCreationDesc& Desc);
    void DestroyInstance();

    bool IsPhysicalDeviceSuitable(VkPhysicalDevice Device, const CPhysicalDeviceFeatures& AvailableFeatures, const CPhysicalDeviceRequestedFeatures& RequestedFeatures, bool StrictCheck = true);
    C_STATUS GetPhysDevice(const CPhysicalDeviceRequestedFeatures& RequestedFeatures, CDeviceHandle& OutHandle);

    bool IsDeviceSuitable(const CPhysicalDevice& PhysDevice, const CDevice& Device, const CPhysicalDeviceRequestedFeatures& RequestedFeatures, VkSurfaceKHR Surface);
    C_STATUS CreateDevice(CPhysicalDevice& PhysDevice, const CDeviceFeaturesInfo& DeviceFeatures, CDeviceHandle& OutHandle);
    void DestroyDevices();
    void DestroyDevice(CDeviceHandle Handle, bool Full = true);
    
    bool CheckInstanceLayersSupport(const Vector<String>& Layers);
    bool CheckInstanceExtensionsSupport(const Vector<String>& Extensions);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice Device, const Vector<String>& PhysicalDeviceExtensions);

    //////////////////////////////////////////////////////////////////////////
    CPhysicalDeviceFeatures GetPhysDeviceFeatures(VkPhysicalDevice PhysDevice, VkSurfaceKHR Surface);
    CPhysicalDeviceRequestedFeatures GetPhysDeviceRequestedFeatures(VkSurfaceKHR Surface);

protected:
    CRenderBackendVulkan* m_BackendVk = nullptr;

    VkInstance m_InstanceVk = VK_NULL_HANDLE;
    CInstanceCreationDesc m_InstanceDesc;

    Vector<CPhysicalDevice> m_PhysDevices;
    Vector<CDevice> m_Devices;
};

} // namespace Cyclone::Render
