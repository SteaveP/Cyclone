#include "TextureVulkan.h"
#include "RenderBackendVulkan.h"
#include "CommandQueueVulkan.h"

#include "Internal/GlobalContextVk.h"

namespace Cyclone::Render
{

CTextureVulkan::CTextureVulkan() = default;
CTextureVulkan::~CTextureVulkan() = default;

Cyclone::C_STATUS CTextureVulkan::Init(const CTextureDesc& Desc)
{
    C_STATUS Result = CTexture::Init(Desc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    m_BackendVk = GET_BACKEND_IMPL(Desc.Backend);


    // #todo_vk also check flag
    if (Desc.ExternalBackendResource)
    {
        m_ImageVk = reinterpret_cast<VkImage>(Desc.ExternalBackendResource);
    }
    else
    {
        const auto& LogicDevice = m_BackendVk->GetGlobalContext().GetLogicalDevice(Desc.DeviceHandle);
        VkImageCreateInfo Info{};
        Info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        Info.pNext = nullptr;

        //VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        //VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        //VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        //VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;
        //VK_IMAGE_CREATE_ALIAS_BIT; // ?
        Info.flags = 0;
        Info.imageType = ConvertImageType(m_Desc.ImageType);
        Info.format = ConvertFormatType(m_Desc.Format);
        Info.extent = { uint32(m_Desc.Width), m_Desc.Height, m_Desc.Depth};
        Info.mipLevels = m_Desc.MipLevels;
        Info.arrayLayers = m_Desc.ArrayCount;
        Info.samples = ConvertSamlpeCountType(m_Desc.SamplesCount);
        Info.tiling = ConvertTilingType(m_Desc.Tiling);
        Info.usage = ConvertTextureUsageType(m_Desc.Usage);
        Info.sharingMode =  VK_SHARING_MODE_EXCLUSIVE; // #todo_vk async compute
        Info.initialLayout = ConvertLayoutType(m_Desc.InitialLayout);
        
        uint32_t QueueFamilyIndices[] = { LogicDevice.GetCommandQueue(CommandQueueType::Graphics)->GetQueueFamilyIndex() };
        Info.queueFamilyIndexCount = 1;
        Info.pQueueFamilyIndices = QueueFamilyIndices;

        VmaAllocationCreateInfo AllocInfo{};
        AllocInfo.usage = VMA_MEMORY_USAGE_AUTO; // #todo_vk refactor?

        VkResult Result = vmaCreateImage(LogicDevice.Allocator, &Info, &AllocInfo, &m_ImageVk, &m_Allocation, &m_AllocationInfo);
        C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
    }

#if ENABLE_DEBUG_RENDER_BACKEND
    if (m_Desc.Name.empty() == false)
    {
        const auto& LogicDevice = m_BackendVk->GetGlobalContext().GetLogicalDevice(m_Desc.DeviceHandle);

        if (m_Allocation != VK_NULL_HANDLE)
        {
            vmaSetAllocationName(LogicDevice.Allocator, m_Allocation, ("Memory " + m_Desc.Name).c_str());
        }

        if (LogicDevice.pfnSetDebugUtilsObjectNameEXT)
        {
            VkDebugUtilsObjectNameInfoEXT DebugInfo{};
            DebugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            DebugInfo.objectType = VK_OBJECT_TYPE_IMAGE;
            DebugInfo.objectHandle = (uint64_t)m_ImageVk;
            DebugInfo.pObjectName = m_Desc.Name.c_str();
            VkResult Result = LogicDevice.pfnSetDebugUtilsObjectNameEXT(LogicDevice.LogicalDeviceHandle, &DebugInfo);
            C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
        }
    }
#endif

    return C_STATUS::C_STATUS_OK;
}

void CTextureVulkan::DeInit()
{
    if (m_ImageVk)
    {
        const auto& LogicDevice = m_BackendVk->GetGlobalContext().GetLogicalDevice(m_Desc.DeviceHandle);

        vmaDestroyImage(LogicDevice.Allocator, m_ImageVk, m_Allocation);
    }

    CTexture::DeInit();
}


} // namespace Cyclone::Render
