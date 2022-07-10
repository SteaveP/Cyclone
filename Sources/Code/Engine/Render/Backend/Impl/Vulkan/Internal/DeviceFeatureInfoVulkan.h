#pragma once

#include "CommonVulkan.h"

namespace Cyclone::Render
{

struct CSwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR Capabilities;
    Vector<VkSurfaceFormatKHR> Formats;
    Vector<VkPresentModeKHR> PresentModes;
};

struct CQueueFamilyIndices
{
    Optional<uint32> GraphicsFamily;
    Optional<uint32> PresentFamily;
    Optional<uint32> AsyncComputeFamily; // optional

    bool IsComplete() const;

    bool operator==(const CQueueFamilyIndices& Other) const noexcept = default;
};

struct CQueueCreateInfo
{
    CommandQueueType Type = CommandQueueType::Graphics;
    uint32 QueueFamilyIndex = 0;
    uint32 QueueCount = 1;
    float Priority = 1.f;
};

struct CDeviceFeaturesInfo
{
    VkSurfaceKHR Surface = VK_NULL_HANDLE;

    // Queues
    Vector<CQueueCreateInfo> Queues;

    // Features
    VkPhysicalDeviceFeatures2 DeviceFeatures2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    VkPhysicalDeviceVulkan12Features DeviceFeatures12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    VkPhysicalDeviceVulkan13Features DeviceFeatures13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };

    // Extensions
    Vector<String> Extensions;
};

struct CPhysicalDeviceFeatures
{
    // Swapchain support for specific Surface
    VkSurfaceKHR Surface = VK_NULL_HANDLE;
    CSwapChainSupportDetails SwapChainSupport;
    CQueueFamilyIndices QueueFamilyIndices;

    VkPhysicalDeviceProperties2 DeviceProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    VkPhysicalDeviceDriverProperties DeviceDriverProperties { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES };

    VkPhysicalDeviceFeatures DeviceFeatures;
    VkPhysicalDeviceFeatures2 DeviceFeatures2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    VkPhysicalDeviceVulkan12Features DeviceFeatures12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    VkPhysicalDeviceVulkan13Features DeviceFeatures13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    Vector<String> AvailableExtensions;
};

// Requested features 
class IRequestedFeature
{
public:
    DISABLE_COPY_ENABLE_MOVE(IRequestedFeature);
    IRequestedFeature(String Name) : m_Name(MoveTemp(Name)) {}
    virtual ~IRequestedFeature() = default;

    virtual bool IsSupported(VkPhysicalDevice PhysDevice, const CPhysicalDeviceFeatures& PhysDeviceFeatures) const = 0;
    virtual bool IsOptional() const = 0;

    virtual void FillRequestStructure(const CPhysicalDevice& PhysDevice, CDeviceFeaturesInfo& ReqFeatures) const = 0;

    virtual void SetEnabled(bool Value) { m_Enabled = Value; }

    const String& GetName() const { return m_Name; }
    bool IsEnabled() const { return m_Enabled; }

protected:
    String m_Name;
    bool m_Enabled = true;
};

struct CPhysicalDeviceRequestedFeatures
{
    VkSurfaceKHR Surface = VK_NULL_HANDLE;

    Vector<Ptr<IRequestedFeature>> RequestedFeatures;

    bool IsSuitable(VkPhysicalDevice PhysDevice, const CPhysicalDeviceFeatures& AvailableFeatures, bool StrictCheck = true) const;
    void Fill(const CPhysicalDevice& PhysDevice, CDeviceFeaturesInfo& Features) const;
};

} // namespace Cyclone::Render
