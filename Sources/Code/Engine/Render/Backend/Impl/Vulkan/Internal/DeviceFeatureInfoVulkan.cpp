#include "DeviceManagerVulkan.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"

#include "Engine/Utils/Config.h"
#include "Engine/Utils/Log.h"

#include "RenderBackendVulkan.h"

#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"

namespace Cyclone::Render
{

// #todo_vk #todo_vk_shader for all requested features need to add support of shader extensions and related permutations

class CGeneralRequestedFeatures : public IRequestedFeature
{
public:
    CGeneralRequestedFeatures(Vector<String> GeneralExtensions) : IRequestedFeature("General"), m_GeneralExtensions(MoveTemp(GeneralExtensions))
    {}
    virtual bool IsOptional() const override { return false; }

    virtual bool IsSupported(VkPhysicalDevice PhysDevice, const CPhysicalDeviceFeatures& PhysDeviceFeatures) const override
    {
        const auto& Avail = PhysDeviceFeatures.AvailableExtensions;
        for (const auto& Ext : m_GeneralExtensions)
        {
            if (std::find(Avail.begin(), Avail.end(), Ext) == Avail.end())
            {
                return false;
            }
        }

        return PhysDeviceFeatures.DeviceFeatures.samplerAnisotropy
            && PhysDeviceFeatures.DeviceFeatures12.timelineSemaphore
            && PhysDeviceFeatures.DeviceFeatures13.dynamicRendering
            && PhysDeviceFeatures.DeviceFeatures13.synchronization2;
    }

    virtual void FillRequestStructure(const CPhysicalDevice& PhysDevice, CDeviceFeaturesInfo& ReqFeatures) const override
    {
        ReqFeatures.DeviceFeatures2.features.samplerAnisotropy = true;
        ReqFeatures.DeviceFeatures12.timelineSemaphore = true;
        ReqFeatures.DeviceFeatures13.dynamicRendering = true;
        ReqFeatures.DeviceFeatures13.synchronization2 = true;

        // #todo_vk
        //ReqFeatures.DeviceFeatures13.privateData = true;
        //ReqFeatures.DeviceFeatures12.bufferDeviceAddressCaptureReplay = true;
        //ReqFeatures.DeviceFeatures12.bufferDeviceAddress = true;

        ReqFeatures.Extensions.insert(ReqFeatures.Extensions.end(), m_GeneralExtensions.begin(), m_GeneralExtensions.end());
    }

protected:
    Vector<String> m_GeneralExtensions;
};

class CSwapChainRequestedFeature : public IRequestedFeature
{
public:
    CSwapChainRequestedFeature() : IRequestedFeature("Swapchain") {}
    virtual bool IsOptional() const override { return false; }

    virtual bool IsSupported(VkPhysicalDevice PhysDevice, const CPhysicalDeviceFeatures& PhysDeviceFeatures) const override
    {
        const auto& Exts = PhysDeviceFeatures.AvailableExtensions;
        bool ExtentionsSupported = std::find(Exts.begin(), Exts.end(), 
            String(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) != Exts.end();

        return ExtentionsSupported &&
            PhysDeviceFeatures.Surface &&
            !PhysDeviceFeatures.SwapChainSupport.Formats.empty() &&
            !PhysDeviceFeatures.SwapChainSupport.PresentModes.empty();
    }

    virtual void FillRequestStructure(const CPhysicalDevice& PhysDevice, CDeviceFeaturesInfo& ReqFeatures) const override
    {
        float QueuePriority = 1.f;

        {
            auto& QueueInfo = ReqFeatures.Queues.emplace_back();
            QueueInfo.Type = CommandQueueType::Graphics;
            QueueInfo.Priority = QueuePriority;
            QueueInfo.QueueCount = 1;
            QueueInfo.QueueFamilyIndex = PhysDevice.Features.QueueFamilyIndices.GraphicsFamily.value();
        }
        {
            auto& QueueInfo = ReqFeatures.Queues.emplace_back();
            QueueInfo.Type = CommandQueueType::Present;
            QueueInfo.Priority = QueuePriority;
            QueueInfo.QueueCount = 1;
            QueueInfo.QueueFamilyIndex = PhysDevice.Features.QueueFamilyIndices.PresentFamily.value();
        }

        ReqFeatures.Extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
};

class CBindlessRequestedFeature : public IRequestedFeature
{
public:
    DISABLE_COPY_ENABLE_MOVE(CBindlessRequestedFeature);
    CBindlessRequestedFeature(uint32* FeatureFlagPtr) : IRequestedFeature("Bindless"), m_FeatureFlagPtr(FeatureFlagPtr) {}
    virtual bool IsOptional() const override { return false; } // #todo_vk make this optional

    virtual bool IsSupported(VkPhysicalDevice PhysDevice, const CPhysicalDeviceFeatures& PhysDeviceFeatures) const override;
    virtual void FillRequestStructure(const CPhysicalDevice& PhysDevice, CDeviceFeaturesInfo& ReqFeatures) const override;

    virtual void SetEnabled(bool Value) override { IRequestedFeature::SetEnabled(Value); if (m_FeatureFlagPtr) { *m_FeatureFlagPtr = Value; } }

protected:
    uint32* m_FeatureFlagPtr = nullptr;
};

class CAsyncComputeRequestFeature : public IRequestedFeature
{
public:
    CAsyncComputeRequestFeature(uint32* FeatureFlagPtr) : IRequestedFeature("AsyncCompute"), m_FeatureFlagPtr(FeatureFlagPtr) {}
    virtual bool IsOptional() const override { return true; }

    virtual bool IsSupported(VkPhysicalDevice PhysDevice, const CPhysicalDeviceFeatures& PhysDeviceFeatures) const override
    {
        return PhysDeviceFeatures.QueueFamilyIndices.AsyncComputeFamily.has_value();  // #todo_vk_async_compute check that it is different from Graphics Family
    }

    virtual void FillRequestStructure(const CPhysicalDevice& PhysDevice, CDeviceFeaturesInfo& ReqFeatures) const override
    {
        auto& QueueInfo = ReqFeatures.Queues.emplace_back();
        QueueInfo.Type = CommandQueueType::AsyncCompute;
        QueueInfo.Priority = 1.f;
        QueueInfo.QueueCount = 1;
        QueueInfo.QueueFamilyIndex = PhysDevice.Features.QueueFamilyIndices.AsyncComputeFamily.value();
    }

    virtual void SetEnabled(bool Value) override { IRequestedFeature::SetEnabled(Value); if (m_FeatureFlagPtr) { *m_FeatureFlagPtr = Value; } }

protected:
    uint32* m_FeatureFlagPtr = nullptr;
};


class CPreferredGPURequestedFeature : public IRequestedFeature
{
public:
    CPreferredGPURequestedFeature(CRenderBackendVulkan* BackendVk, String PreferredGPU) : IRequestedFeature("PreferredGPU"),
        m_BackendVk(BackendVk), m_IsOptional(true), m_PreferredGPU(PreferredGPU)
    {
        // If try to run on specific GPU index, this request isn't optional
        if (PreferredGPU.starts_with("GPU"))
            m_IsOptional = false;
    }
    virtual bool IsOptional() const override { return m_IsOptional; }

    virtual bool IsSupported(VkPhysicalDevice PhysDevice, const CPhysicalDeviceFeatures& PhysDeviceFeatures) const override
    {
        const auto& DeviceType = PhysDeviceFeatures.DeviceProperties.properties.deviceType;
        const auto& DriverType = PhysDeviceFeatures.DeviceDriverProperties.driverID;

        if (m_PreferredGPU == "None")
            return true;
        else if (m_PreferredGPU == "Discrete")
            return DeviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        else if (m_PreferredGPU == "Integrated")
            return DeviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
        else if (m_PreferredGPU == "AMD")
            return DriverType == VK_DRIVER_ID_AMD_OPEN_SOURCE || DriverType == VK_DRIVER_ID_AMD_PROPRIETARY;
        else if (m_PreferredGPU == "INTEL")
            return DriverType == VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS || DriverType == VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA;
        else if (m_PreferredGPU == "NVIDIA")
            return DriverType == VK_DRIVER_ID_NVIDIA_PROPRIETARY;
        else if (m_PreferredGPU.starts_with("GPU"))
        {
            uint32 DeviceIndex = 0;
            if (ConvertFromString(m_PreferredGPU.substr(3), DeviceIndex))
            {
                CASSERT(m_BackendVk);

                uint32 DeviceCount = 0;
                vkEnumeratePhysicalDevices(m_BackendVk->GetDeviceManager().GetInstanceVk(), &DeviceCount, nullptr);

                C_ASSERT_RETURN_VAL(DeviceIndex < DeviceCount, false);

                Vector<VkPhysicalDevice> Devices(DeviceCount);
                vkEnumeratePhysicalDevices(m_BackendVk->GetDeviceManager().GetInstanceVk(), &DeviceCount, &Devices[0]);

                return PhysDevice == Devices[DeviceIndex];
            }
            else
            {
                CASSERT(false);
                return false;
            }
        }

        return false;
    }

    virtual void FillRequestStructure(const CPhysicalDevice& PhysDevice, CDeviceFeaturesInfo& ReqFeatures) const override
    {
        // nothing to do
    }

protected:
    CRenderBackendVulkan* m_BackendVk = nullptr;
    bool m_IsOptional = true;

    String m_PreferredGPU;
};

bool CBindlessRequestedFeature::IsSupported(VkPhysicalDevice PhysDevice, const CPhysicalDeviceFeatures& PhysDeviceFeatures) const
{
    const auto& Feat12 = PhysDeviceFeatures.DeviceFeatures12;
    bool Result = Feat12.descriptorIndexing
        && Feat12.runtimeDescriptorArray
        && Feat12.descriptorBindingPartiallyBound
        // && Feat2.descriptorBindingUniformBufferUpdateAfterBind
        &&Feat12.descriptorBindingSampledImageUpdateAfterBind
        && Feat12.descriptorBindingStorageImageUpdateAfterBind
        && Feat12.descriptorBindingStorageBufferUpdateAfterBind
        && Feat12.descriptorBindingUniformTexelBufferUpdateAfterBind
        && Feat12.descriptorBindingStorageTexelBufferUpdateAfterBind;

    return Result;
}

void CBindlessRequestedFeature::FillRequestStructure(const CPhysicalDevice& PhysDevice, CDeviceFeaturesInfo& ReqFeatures) const
{
    auto& Feat12 = ReqFeatures.DeviceFeatures12;
    Feat12.descriptorIndexing = true;
    Feat12.runtimeDescriptorArray = true;
    Feat12.descriptorBindingPartiallyBound = true;
    //Feat12.descriptorBindingUniformBufferUpdateAfterBind = true;
    Feat12.descriptorBindingSampledImageUpdateAfterBind = true;
    Feat12.descriptorBindingStorageImageUpdateAfterBind = true;
    Feat12.descriptorBindingStorageBufferUpdateAfterBind = true;
    Feat12.descriptorBindingUniformTexelBufferUpdateAfterBind = true;
    Feat12.descriptorBindingStorageTexelBufferUpdateAfterBind = true;
}

bool CQueueFamilyIndices::IsComplete() const
{
    return GraphicsFamily.has_value() && PresentFamily.has_value() && AsyncComputeFamily.has_value();
}

CQueueFamilyIndices CDeviceManagerVulkan::FindQueueFamilies(VkPhysicalDevice Device, VkSurfaceKHR Surface) const
{
    CQueueFamilyIndices Indices{};

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
        
        if (!Indices.AsyncComputeFamily.has_value() && QueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            Indices.AsyncComputeFamily = i;
        }

        if (Indices.IsComplete())
            break;

        i++;
    }

    return Indices;
}

CSwapChainSupportDetails CDeviceManagerVulkan::QuerySwapChainSupport(VkPhysicalDevice Device, VkSurfaceKHR Surface) const
{
    CSwapChainSupportDetails Details{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface, &Details.Capabilities);

    uint32_t FormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, nullptr);

    if (FormatCount != 0)
    {
        Details.Formats.resize(FormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, Details.Formats.data());
    }

    uint32_t PresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, nullptr);

    if (PresentModeCount != 0)
    {
        Details.PresentModes.resize(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, Details.PresentModes.data());
    }

    return Details;
}

CPhysicalDeviceRequestedFeatures CDeviceManagerVulkan::GetPhysDeviceRequestedFeatures(VkSurfaceKHR Surface)
{
    CPhysicalDeviceRequestedFeatures ReqFeatures{};

    ReqFeatures.Surface = Surface;

    Vector<String> RequestedExtensions;
    RequestedExtensions = GET_CONFIG_VULKAN().value("DeviceExtensions", RequestedExtensions);
    ReqFeatures.RequestedFeatures.emplace_back(MakeShared<CGeneralRequestedFeatures>(MoveTemp(RequestedExtensions)));
    
    ReqFeatures.RequestedFeatures.emplace_back(MakeShared<CSwapChainRequestedFeature>());

    String PreferredGPU = "Discrete";
    PreferredGPU = GET_CONFIG_RENDER().value("PreferredGPU", PreferredGPU);
    GStartupArguments()->GetParameter("-PreferredGPU", PreferredGPU);
    ReqFeatures.RequestedFeatures.emplace_back(MakeShared<CPreferredGPURequestedFeature>(m_BackendVk, PreferredGPU));

    if (GEnableBindless)
    {
        ReqFeatures.RequestedFeatures.emplace_back(
            MakeShared<CBindlessRequestedFeature>(&GEnableBindless));
    }
    if (GEnableAsyncCompute)
    {
        ReqFeatures.RequestedFeatures.emplace_back(
            MakeShared<CAsyncComputeRequestFeature>(&GEnableAsyncCompute));
    }

    return ReqFeatures;
}

bool CPhysicalDeviceRequestedFeatures::IsSuitable(VkPhysicalDevice PhysDevice, const CPhysicalDeviceFeatures& AvailableFeatures, bool StrictCheck) const
{
    for (uint32 i = 0; i < (uint32)RequestedFeatures.size(); ++i)
    {
        auto& ReqFeature = RequestedFeatures[i];

        bool Result = ReqFeature->IsSupported(PhysDevice, AvailableFeatures);

        if (Result == false && (StrictCheck || !ReqFeature->IsOptional()))
        {
            LOG_INFO("VK: IsSuitable check failed for DeviceFeature {} for PhysDevice \"{}\"",
                ReqFeature->GetName(), AvailableFeatures.DeviceProperties.properties.deviceName);
            return false;
        }
    }

    return true;
}

void CPhysicalDeviceRequestedFeatures::Fill(const CPhysicalDevice& PhysDevice, CDeviceFeaturesInfo& Features) const
{
    Features.Surface = PhysDevice.RequestedFeatures.Surface;

    for (uint32 i = 0; i < (uint32)RequestedFeatures.size(); ++i)
    {
        auto& ReqFeature = RequestedFeatures[i];
        if (ReqFeature->IsEnabled())
            ReqFeature->FillRequestStructure(PhysDevice,Features);
    }
}

} // namespace Cyclone::Render
