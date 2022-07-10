#include "DeviceManagerVulkan.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"
#include "Engine/Utils/Config.h"
#include "Engine/Utils/Delegate.h"
#include "Engine/Render/Backend/UploadQueue.h"

#include "RenderBackendVulkan.h"

#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"

namespace Cyclone::Render
{

// #todo_vk move to common rendering and common vulkan files
// Features
uint32 GEnableBindless = true;
uint32 GEnableAsyncCompute = false;
uint32 GEnableDynamicRendering = true; // #todo_vk implement it, but not supported on Intel U620 integrated videocars (need to check AMD Ryzen 4800)

// Debug/Validation
uint32 GEnableValidation = true;
uint32 GEnableGPUValidation = false;
uint32 GEnableRenderDocVk = false;

#ifdef _DEBUG
    const bool GAllowValidation = true;
#else
    const bool GAllowValidation = false;
#endif

static Vector<const char*> GInstanceExtensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,

#if PLATFORM_WIN64
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
    #error unsupported platform
#endif

#if ENABLE_DEBUG_RENDER_BACKEND
     VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

CDeviceManagerVulkan::CDeviceManagerVulkan() = default;
CDeviceManagerVulkan::~CDeviceManagerVulkan()
{
    CASSERT(m_InstanceVk == VK_NULL_HANDLE);
    CASSERT(m_Devices.size() == 0);
}

C_STATUS CDeviceManagerVulkan::Init(CRenderBackendVulkan* RenderBackend, const CInstanceCreationDesc& Desc)
{
    m_BackendVk = RenderBackend;

    GEnableAsyncCompute = GET_CONFIG_RENDER().value("EnableAsyncCompute", GEnableAsyncCompute);
    GEnableBindless = GET_CONFIG_RENDER().value("EnableBindless", GEnableBindless);

    GEnableValidation = GET_CONFIG_RENDER().value("EnableValidation", GEnableValidation);
    GEnableGPUValidation = GET_CONFIG_RENDER().value("EnableGPUValidation", GEnableGPUValidation);
    GEnableRenderDocVk = GET_CONFIG_RENDER().value("EnableRenderdoc", GEnableRenderDocVk);

#if ENABLE_VOLK_LOADER
    VkResult ResultVk = volkInitialize();
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);
#endif

    C_STATUS Result = CreateInstance(Desc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CDeviceManagerVulkan::Shutdown()
{
    DestroyDevices();
    DestroyInstance();

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CDeviceManagerVulkan::CreateInstance(const CInstanceCreationDesc& Desc)
{
    m_InstanceDesc = Desc;

    if (GEnableValidation && GAllowValidation)
    {
        Vector<String> DataFromConfig;
        DataFromConfig = GET_CONFIG_VULKAN().value("ValidationInstanceLayers", DataFromConfig);
        for (uint32 i = 0; i < (uint32)DataFromConfig.size(); ++i)
        {
            m_InstanceDesc.EnabledLayers.emplace_back(MoveTemp(DataFromConfig[i]));
        }

        DataFromConfig = GET_CONFIG_VULKAN().value("ValidationInstanceExtensions", DataFromConfig);
        for (uint32 i = 0; i < (uint32)DataFromConfig.size(); ++i)
        {
            m_InstanceDesc.EnabledExtensions.emplace_back(MoveTemp(DataFromConfig[i]));
        }
    }
    if (GEnableRenderDocVk)
    {
        m_InstanceDesc.EnabledLayers.emplace_back("VK_LAYER_RENDERDOC_Capture");
    }

    for (uint32 i = 0; i < (uint32)GInstanceExtensions.size(); ++i)
    {
        m_InstanceDesc.EnabledExtensions.emplace_back(GInstanceExtensions[i]);
    }

    if (CheckInstanceLayersSupport(m_InstanceDesc.EnabledLayers) == false)
    {
        C_ASSERT_RETURN_VAL(FALSE, C_STATUS::C_STATUS_INVALID_ARG);
    }

    if (CheckInstanceExtensionsSupport(m_InstanceDesc.EnabledExtensions) == false)
    {
        C_ASSERT_RETURN_VAL(FALSE, C_STATUS::C_STATUS_INVALID_ARG);
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
    
    Vector<const char*> InstanceLayers(m_InstanceDesc.EnabledLayers.size());
    for (uint32 i = 0; i < (uint32)m_InstanceDesc.EnabledLayers.size(); ++i)
    {
        InstanceLayers[i] = m_InstanceDesc.EnabledLayers[i].c_str();
    }
    CreateInfo.ppEnabledLayerNames = InstanceLayers.data();
    CreateInfo.enabledLayerCount = static_cast<uint32_t>(InstanceLayers.size());

    Vector<const char*> InstanceExtensions(m_InstanceDesc.EnabledExtensions.size());
    for (uint32 i = 0; i < (uint32)m_InstanceDesc.EnabledExtensions.size(); ++i)
    {
        InstanceExtensions[i] = m_InstanceDesc.EnabledExtensions[i].c_str();
    }
    CreateInfo.ppEnabledExtensionNames = InstanceExtensions.data();
    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());

    // GPU Validation feature
    VkValidationFeaturesEXT GPUValidationDesc{};
    Array<VkValidationFeatureEnableEXT, 10> GPUValidationFeatures{};
    if (GEnableGPUValidation && GAllowValidation)
    {
        GPUValidationDesc.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;

        auto& Count = GPUValidationDesc.enabledValidationFeatureCount;
        GPUValidationFeatures[Count++] = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT;
        GPUValidationFeatures[Count++] = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT;
        GPUValidationFeatures[Count++] = VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT;
        //GPUValidationFeatures[Count++] = VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT;
        GPUValidationFeatures[Count++] = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;
        GPUValidationDesc.pEnabledValidationFeatures = GPUValidationFeatures.data();

        CASSERT(CreateInfo.pNext == nullptr);
        CreateInfo.pNext = &GPUValidationDesc;
    }

    VkResult Result = vkCreateInstance(&CreateInfo, nullptr, &m_InstanceVk);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

#if ENABLE_VOLK_LOADER
    volkLoadInstanceOnly(m_InstanceVk);
#endif

    return C_STATUS::C_STATUS_OK;
}

void CDeviceManagerVulkan::DestroyInstance()
{
    if (m_InstanceVk != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_InstanceVk, nullptr);
        m_InstanceVk = VK_NULL_HANDLE;
    }
}

bool CDeviceManagerVulkan::CheckInstanceLayersSupport(const Vector<String>& Layers)
{
    uint32_t LayerCount;
    vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

    Vector<VkLayerProperties> AvailableLayers(LayerCount);
    vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

    for (const auto& LayerName : Layers)
    {
        bool LayerFound = false;

        for (const auto& LayerProperties : AvailableLayers)
        {
            if (stricmp(LayerName.c_str(), LayerProperties.layerName) == 0)
            {
                LayerFound = true;
                break;
            }
        }

        if (!LayerFound)
            return false;
    }

    return true;
}

bool CDeviceManagerVulkan::CheckInstanceExtensionsSupport(const Vector<String>& Extensions)
{
    uint32_t ExtensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);

    Vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, AvailableExtensions.data());

    for (const auto& ExtensionName : Extensions)
    {
        bool ExtensionFound = false;

        for (const auto& ExtensionProperties : AvailableExtensions)
        {
            if (stricmp(ExtensionName.c_str(), ExtensionProperties.extensionName) == 0)
            {
                ExtensionFound = true;
                break;
            }
        }

        if (!ExtensionFound)
            return false;
    }

    return true;
}

C_STATUS CDeviceManagerVulkan::GetOrCreateDevice(VkSurfaceKHR Surface, CDeviceHandle& OutHandle)
{
    OutHandle = CDeviceHandle{};

    CPhysicalDeviceRequestedFeatures RequestedFeatures = GetPhysDeviceRequestedFeatures(Surface);

    // Check cache for physical device first with strict checks
    bool StrictCheck = true;
    for (uint32 i = 0; i < m_PhysDevices.size(); ++i)
    {
        const auto& PhysDevice = m_PhysDevices[i];
        if (IsPhysicalDeviceSuitable(PhysDevice.DeviceVk, PhysDevice.Features, RequestedFeatures, StrictCheck))
        {
            OutHandle.PhysDeviceHandle = i;
            break;
        }
    }

    // Create physical device otherwise
    C_STATUS Result = C_STATUS::C_STATUS_OK;

    if (!OutHandle.IsPhysDeviceHandleValid())
    {
        Result = GetPhysDevice(RequestedFeatures, OutHandle);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    CPhysicalDevice& PhysDevice = m_PhysDevices[OutHandle.PhysDeviceHandle];

    // Check cache for device
    for (uint32 j = 0; j < m_Devices.size(); ++j)
    {
        auto& Device = m_Devices[j];
        if (IsDeviceSuitable(PhysDevice, Device, RequestedFeatures, Surface))
        {
            OutHandle.DeviceHandle = j;
            Device.ReferenceCount++;

            C_ASSERT_RETURN_VAL(OutHandle.IsValid(), C_STATUS::C_STATUS_ERROR);
            return C_STATUS::C_STATUS_OK;
        }
    }

    // Create device otherwise
    {
        CDeviceFeaturesInfo DeviceFeatures{};
        PhysDevice.RequestedFeatures.Fill(PhysDevice, DeviceFeatures);
        Result = CreateDevice(PhysDevice, DeviceFeatures, OutHandle);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

        if (m_BackendVk->GetOnDeviceCreatedDelegate() && *m_BackendVk->GetOnDeviceCreatedDelegate())
            (*m_BackendVk->GetOnDeviceCreatedDelegate())(OutHandle);
    }

    C_ASSERT_RETURN_VAL(OutHandle.IsValid(), C_STATUS::C_STATUS_ERROR);
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CDeviceManagerVulkan::ReleaseDevice(CDeviceHandle Handle)
{
    CASSERT(Handle.IsValid());

    auto& Device = m_Devices[Handle.DeviceHandle];
    CASSERT(Device.ReferenceCount > 0);

    if (--Device.ReferenceCount == 0)
    {
        if (m_BackendVk->GetOnDeviceRemovedDelegate() && *m_BackendVk->GetOnDeviceRemovedDelegate())
            (*m_BackendVk->GetOnDeviceRemovedDelegate())(Handle);

        // #todo_vk need to safely delete device + increase generation
        //DestroyDevice(Handle);
    }

    // No need to remove physical device because it just enumerated, not created

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CDeviceManagerVulkan::GetPhysDevice(const CPhysicalDeviceRequestedFeatures& RequestedFeatures, CDeviceHandle& OutHandle)
{
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(GetInstanceVk(), &DeviceCount, nullptr);
    C_ASSERT_RETURN_VAL(DeviceCount != 0, C_STATUS::C_STATUS_ERROR);

    Vector<VkPhysicalDevice> PhysDevices(DeviceCount);
    vkEnumeratePhysicalDevices(GetInstanceVk(), &DeviceCount, PhysDevices.data());

    auto FindDevice = [&](bool StrictCheck)
    {
        for (int32 i = 0; i < (int32)DeviceCount; ++i)
        {
            VkPhysicalDevice PhysDeviceVk = PhysDevices[i];

            CPhysicalDeviceFeatures Features = GetPhysDeviceFeatures(PhysDeviceVk, RequestedFeatures.Surface);

            if (IsPhysicalDeviceSuitable(PhysDeviceVk, Features, RequestedFeatures, true))
            {
                return i;
            }
        }

        return -1;
    };

    int32 ChoosenDevice = FindDevice(true);

    // If can't choose device with all requested features
    // allow to setup with unsupported features disabled
    if (ChoosenDevice < 0)
        ChoosenDevice = FindDevice(false);

    C_ASSERT_RETURN_VAL(ChoosenDevice >= 0, C_STATUS::C_STATUS_INVALID_ARG);

    OutHandle.PhysDeviceHandle = static_cast<uint16>(m_Devices.size());
    auto& PhysDevice = m_PhysDevices.emplace_back();

    PhysDevice.DeviceVk = PhysDevices[ChoosenDevice];
    PhysDevice.ID = OutHandle.PhysDeviceHandle;
    PhysDevice.Features = GetPhysDeviceFeatures(PhysDevice.DeviceVk, RequestedFeatures.Surface);
    PhysDevice.Name = PhysDevice.Features.DeviceProperties.properties.deviceName;

    // Disable unsupported optional features
    PhysDevice.RequestedFeatures = RequestedFeatures;
    for (uint32 i = 0; i < (uint32)PhysDevice.RequestedFeatures.RequestedFeatures.size(); ++i)
    {
        auto& ReqFeature = PhysDevice.RequestedFeatures.RequestedFeatures[i];
        if (ReqFeature->IsSupported(PhysDevice.DeviceVk, PhysDevice.Features) == false)
        {
            CASSERT(ReqFeature->IsOptional());
            ReqFeature->SetEnabled(false);
        }
    }

    return C_STATUS::C_STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////

C_STATUS CDeviceManagerVulkan::CreateDevice(CPhysicalDevice& PhysDevice, const CDeviceFeaturesInfo& DeviceFeatures, CDeviceHandle& OutHandle)
{
    CASSERT(m_Devices.size() <= 0 && "Multi-devices aren't tested yet"); // #todo_vk

    OutHandle.DeviceHandle = static_cast<uint16>(m_Devices.size());
    CDevice& Device = m_Devices.emplace_back();
    Device.DeviceHandle = OutHandle;
    Device.RequestedFeatures = DeviceFeatures;
    Device.ReferenceCount = 1;

    VkDeviceCreateInfo CreateInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    // Features
    CreateInfo.pNext = &Device.RequestedFeatures.DeviceFeatures2;
    Device.RequestedFeatures.DeviceFeatures2.pNext = &Device.RequestedFeatures.DeviceFeatures12;
    Device.RequestedFeatures.DeviceFeatures12.pNext = &Device.RequestedFeatures.DeviceFeatures13;

    // #todo_vk_refactor this needs to be refactored
    CASSERT(PhysDevice.Features.DeviceProperties.properties.limits.minStorageBufferOffsetAlignment == 16);

    // Queues
    Array<float, 10> QueuePriorities{};
    Vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
    for (uint32 i = 0; i < (uint32)DeviceFeatures.Queues.size(); ++i)
    {
        const auto& QueueInfo = DeviceFeatures.Queues[i];
        auto it = std::find_if(QueueCreateInfos.begin(), QueueCreateInfos.end(), [&QueueInfo](const auto& Info) {
            return Info.queueFamilyIndex == QueueInfo.QueueFamilyIndex;
        });

        if (it == QueueCreateInfos.end())
        {
            uint32 Index = (uint32)QueueCreateInfos.size();
            QueuePriorities[Index] = QueueInfo.Priority;

            auto& QueueCreateInfo = QueueCreateInfos.emplace_back();
            QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            QueueCreateInfo.queueFamilyIndex = QueueInfo.QueueFamilyIndex;
            QueueCreateInfo.queueCount = 1;
            QueueCreateInfo.pQueuePriorities = &QueuePriorities[Index];
        }
        else
        {
            CASSERT(it->queueCount == 1 && it->queueCount == QueueInfo.QueueCount);
        }
    }
    CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
    CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());

    // Extensions
    Vector<const char*> DeviceExtensions(DeviceFeatures.Extensions.size());
    for (uint32 i = 0; i < (uint32)DeviceFeatures.Extensions.size(); ++i)
    {
        DeviceExtensions[i] = DeviceFeatures.Extensions[i].c_str();
    }
    CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();
    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());

    // Create device
    VkResult ResultVk = vkCreateDevice(PhysDevice.DeviceVk, &CreateInfo, nullptr, &Device.DeviceVk);
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);

#if ENABLE_VOLK_LOADER
    volkLoadDeviceTable(&Device.DeviceTable, Device.DeviceVk);
#endif

#if ENABLE_DEBUG_RENDER_BACKEND
    if (Device.DeviceTableExt.vkSetDebugUtilsObjectNameEXT == nullptr)
    {
        Device.DeviceTableExt.vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)
            vkGetDeviceProcAddr(Device.DeviceVk, "vkSetDebugUtilsObjectNameEXT");
    }
    if (Device.DeviceTableExt.vkCmdBeginDebugUtilsLabelEXT == nullptr)
    {
        Device.DeviceTableExt.vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)
            vkGetDeviceProcAddr(Device.DeviceVk, "vkCmdBeginDebugUtilsLabelEXT");
    }
    if (Device.DeviceTableExt.vkCmdEndDebugUtilsLabelEXT == nullptr)
    {
        Device.DeviceTableExt.vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)
            vkGetDeviceProcAddr(Device.DeviceVk, "vkCmdEndDebugUtilsLabelEXT");
    }
    if (Device.DeviceTableExt.vkCmdInsertDebugUtilsLabelEXT == nullptr)
    {
        Device.DeviceTableExt.vkCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)
            vkGetDeviceProcAddr(Device.DeviceVk, "vkCmdInsertDebugUtilsLabelEXT");
    }

    if (Device.DeviceTableExt.vkQueueBeginDebugUtilsLabelEXT == nullptr)
    {
        Device.DeviceTableExt.vkQueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)
            vkGetDeviceProcAddr(Device.DeviceVk, "vkQueueBeginDebugUtilsLabelEXT");
    }
    if (Device.DeviceTableExt.vkQueueEndDebugUtilsLabelEXT == nullptr)
    {
        Device.DeviceTableExt.vkQueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)
            vkGetDeviceProcAddr(Device.DeviceVk, "vkQueueEndDebugUtilsLabelEXT");
    }
#endif // ENABLE_DEBUG_RENDER_BACKEND

    auto InitQueue = [&](CommandQueueType QueueType, uint32_t QueueFamilyIndex, uint32_t QueueIndex)
    {
        UniquePtr<CCommandQueueVulkan> CommandQueue = MakeUnique<CCommandQueueVulkan>();
        C_STATUS res = CommandQueue->Init(m_BackendVk, OutHandle, QueueType, QueueFamilyIndex, QueueIndex);
        CASSERT(C_SUCCEEDED(res));

        CASSERT(Device.CommandQueues[(uint32_t)QueueType] == nullptr);
        Device.CommandQueues[(uint32_t)QueueType] = MoveTemp(CommandQueue);
    };

    for (uint32 i = 0; i < DeviceFeatures.Queues.size(); ++i)
    {
        const auto& QueueInfo = DeviceFeatures.Queues[i];
        InitQueue(QueueInfo.Type, QueueInfo.QueueFamilyIndex, 0);
    }

    // Allocator
    {
        VmaAllocatorCreateInfo Info{};
        Info.instance = m_InstanceVk;
        Info.physicalDevice = PhysDevice.DeviceVk;
        Info.device = Device.DeviceVk;
        Info.vulkanApiVersion = VK_API_VERSION_1_3;
        
#if ENABLE_VOLK_LOADER
        VmaVulkanFunctions Functions{};
        Functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        Functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        Info.pVulkanFunctions = &Functions;
#endif
        VkResult Result = vmaCreateAllocator(&Info, &Device.Allocator);
        C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
    }

    // Resource Manager
    {
        CResourceManagerVulkanDesc Desc{
            .BackendVk = m_BackendVk,
            .DeviceHandle = OutHandle
        };
        Device.ResourceManager = MakeUnique<CResourceManagerVulkan>();
        C_STATUS Result = Device.ResourceManager->Init(Desc);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    // Bindless manager // #todo_vk_bindless #todo_vk_material refactor
    {
        CBindlessDescriptorManagerVulkanDesc Desc{};
        Desc.PoolDesc = CDescriptorPoolVkDesc {
            .DescriptorsCount = 100, // #todo_config read values from config
            .Bindless = true,
            .DeviceHandle = OutHandle,
            .BackendVk = m_BackendVk
        };
#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.PoolDesc.Name = "DescriptorPoolBindless";
#endif
        Desc.BackendVk = m_BackendVk;

        Device.BindlessManager = MakeUnique<CBindlessDescriptorManagerVulkan>();
        C_STATUS Result = Device.BindlessManager->Init(Desc);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    // UploadQueue
    {
        CUploadQueueDesc Desc{};
        Desc.Backend = m_BackendVk;
        Desc.DeviceHandle = OutHandle;
        Desc.CommandQueue = Device.GetCommandQueue(CommandQueueType::Graphics);
        Desc.BufferSize = GET_CONFIG_RENDER().value("UploadQueueBufferSize", Desc.BufferSize);

        Device.UploadQueue = MakeUnique<CUploadQueue>();

        C_STATUS Result = Device.UploadQueue->Init(Desc);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    // Disposal Manager
    {
        CDisposalManagerDesc Desc{};

        Device.DisposalManager = MakeUnique<CDisposalManagerVulkan>();
        Device.DisposalManager->Init(m_BackendVk, MoveTemp(Desc));
    }

    return C_STATUS::C_STATUS_OK;
}

bool CDeviceManagerVulkan::IsPhysicalDeviceSuitable(VkPhysicalDevice PhysDevice, const CPhysicalDeviceFeatures& AvailableFeatures, const CPhysicalDeviceRequestedFeatures& RequestedFeatures, bool StrictCheck)
{
    return RequestedFeatures.IsSuitable(PhysDevice, AvailableFeatures, StrictCheck);
}

bool CDeviceManagerVulkan::CheckDeviceExtensionSupport(VkPhysicalDevice PhysDevice, const Vector<String>& PhysicalDeviceExtensions)
{
    uint32_t ExtensionCount;
    vkEnumerateDeviceExtensionProperties(PhysDevice, nullptr, &ExtensionCount, nullptr);

    Vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
    vkEnumerateDeviceExtensionProperties(PhysDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

    Set<String> RequiredExtensions(PhysicalDeviceExtensions.begin(), PhysicalDeviceExtensions.end());

    for (const auto& Extension : AvailableExtensions)
    {
        RequiredExtensions.erase(Extension.extensionName);
    }

    return RequiredExtensions.empty();
}

bool CDeviceManagerVulkan::IsDeviceSuitable(const CPhysicalDevice& PhysDevice, const CDevice& Device, const CPhysicalDeviceRequestedFeatures& RequestedFeatures, VkSurfaceKHR Surface)
{
    // #todo_vk check that requested features of created device
    // are subset of existing device + refactor to pass
    
    CQueueFamilyIndices Indices = FindQueueFamilies(PhysDevice.DeviceVk, Surface);
    if (Indices != PhysDevice.Features.QueueFamilyIndices)
        return false;

    if (Indices.GraphicsFamily.has_value() == false || Indices.PresentFamily.has_value() == false)
        return false;

    VkBool32 IsSupported = false;
    VkResult Result = vkGetPhysicalDeviceSurfaceSupportKHR(PhysDevice.DeviceVk, Indices.PresentFamily.value(), Surface, &IsSupported);
    C_ASSERT_VK_SUCCEEDED_RET(Result, false);

    return IsSupported;
}

void CDeviceManagerVulkan::DestroyDevices()
{
    for (auto& Device : m_Devices)
    {
        DestroyDevice(Device.DeviceHandle);
    }
    m_Devices.clear();

    // No need to do anything with Physical Devices
    m_PhysDevices.clear();
}

void CDeviceManagerVulkan::DestroyDevice(CDeviceHandle Handle, bool Full)
{
    auto& Device = m_Devices[Handle.DeviceHandle];

    // #todo_vk increase generation

    Device.UploadQueue.reset();
    Device.BindlessManager.reset();
    Device.ResourceManager.reset();

    for (auto& Queue : Device.CommandQueues)
    {
        if (Queue)
            Queue->DeInit();

        Queue.reset();
    }

    if(Device.DisposalManager)
        Device.DisposalManager->Tick(true);

    Device.DisposalManager.reset();

    if (Device.Allocator)
    {
        vmaDestroyAllocator(Device.Allocator);
        Device.Allocator = VK_NULL_HANDLE;
    }


    if (Device.DeviceVk)
    {
        VK_CALL(Device, vkDestroyDevice(Device.DeviceVk, nullptr));
        Device.DeviceVk = VK_NULL_HANDLE;
    }

}

CPhysicalDeviceFeatures CDeviceManagerVulkan::GetPhysDeviceFeatures(VkPhysicalDevice PhysDevice, VkSurfaceKHR Surface)
{
    CPhysicalDeviceFeatures Features{};
    Features.Surface = Surface;

    Features.DeviceProperties.pNext = &Features.DeviceDriverProperties;

    vkGetPhysicalDeviceProperties2(PhysDevice, &Features.DeviceProperties);
    vkGetPhysicalDeviceFeatures(PhysDevice, &Features.DeviceFeatures);

    Features.DeviceFeatures2.pNext = &Features.DeviceFeatures12;
    Features.DeviceFeatures12.pNext = &Features.DeviceFeatures13;
    vkGetPhysicalDeviceFeatures2(PhysDevice, &Features.DeviceFeatures2);
    

    Features.SwapChainSupport = QuerySwapChainSupport(PhysDevice, Surface);
    Features.QueueFamilyIndices = FindQueueFamilies(PhysDevice, Surface);

    // Extensions
    {
        uint32_t ExtensionCount;
        vkEnumerateDeviceExtensionProperties(PhysDevice, nullptr, &ExtensionCount, nullptr);

        Vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
        vkEnumerateDeviceExtensionProperties(PhysDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

        Features.AvailableExtensions.resize(ExtensionCount);
        for (uint32 i = 0; i < ExtensionCount; ++i)
        {
            Features.AvailableExtensions[i] = AvailableExtensions[i].extensionName;
        }
    }

    return Features;
}

} // namespace Cyclone::Render
