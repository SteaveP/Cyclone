#include "BufferVulkan.h"
#include "RenderBackendVulkan.h"
#include "CommandQueueVulkan.h"

#include "Internal/GlobalContextVk.h"

namespace Cyclone::Render
{

CBufferVulkan::CBufferVulkan() = default;
CBufferVulkan::~CBufferVulkan() = default;

C_STATUS CBufferVulkan::Init(const CBufferDesc& Desc)
{
    C_STATUS Result = CBuffer::Init(Desc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    m_BackendVk = GET_BACKEND_IMPL(Desc.Backend);

    // #todo_vk also check flag
    if (Desc.ExternalBackendResource)
    {
        m_BufferVk = reinterpret_cast<VkBuffer>(Desc.ExternalBackendResource);
    }
    else
    {
        const auto& LogicDevice = m_BackendVk->GetGlobalContext().GetLogicalDevice(Desc.Device);

        VkBufferCreateInfo Info{};
        Info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        Info.pNext = nullptr;
        Info.flags = 0;
        Info.size = m_Desc.ByteSize;
        Info.usage = ConvertBufferUsageType(m_Desc.Usage);
        Info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // #todo_vk async compute

        uint32_t QueueFamilyIndices[] = { LogicDevice.GetCommandQueue(CommandQueueType::Graphics)->GetQueueFamilyIndex() };
        Info.queueFamilyIndexCount = 1;
        Info.pQueueFamilyIndices = QueueFamilyIndices;

        VmaAllocationCreateInfo AllocInfo{};
        AllocInfo.usage = VMA_MEMORY_USAGE_AUTO; // #todo_vk refactor?

        if (Desc.Flags & EResourceFlags::Mappable)
        {
            // Needed to set VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT flag for memory
            AllocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        }

        VkResult Result = vmaCreateBuffer(LogicDevice.Allocator, &Info, &AllocInfo, &m_BufferVk, &m_Allocation, &m_AllocationInfo);
        C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
    }

#if ENABLE_DEBUG_RENDER_BACKEND
    if (m_Desc.Name.empty() == false)
    {
        const auto& LogicDevice = m_BackendVk->GetGlobalContext().GetLogicalDevice(Desc.Device);

        if (m_Allocation != VK_NULL_HANDLE)
        {
            vmaSetAllocationName(LogicDevice.Allocator, m_Allocation, ("Memory " + m_Desc.Name).c_str());
        }

        if (LogicDevice.pfnSetDebugUtilsObjectNameEXT)
        {
            VkDebugUtilsObjectNameInfoEXT DebugInfo{};
            DebugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            DebugInfo.objectType = VK_OBJECT_TYPE_BUFFER;
            DebugInfo.objectHandle = (uint64_t)m_BufferVk;
            DebugInfo.pObjectName = m_Desc.Name.c_str();
            VkResult Result = LogicDevice.pfnSetDebugUtilsObjectNameEXT(LogicDevice.LogicalDeviceHandle, &DebugInfo);
            C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
        }
    }
#endif

    return C_STATUS::C_STATUS_OK;
}

void CBufferVulkan::DeInit()
{
    if (m_BufferVk)
    {
        const auto& LogicDevice = m_BackendVk->GetGlobalContext().GetLogicalDevice(m_Desc.Device);

        vmaDestroyBuffer(LogicDevice.Allocator, m_BufferVk, m_Allocation);
    }

    CBuffer::DeInit();
}

CMapData CBufferVulkan::Map()
{
    CMapData MapData{};

    const auto& LogicDevice = m_BackendVk->GetGlobalContext().GetLogicalDevice(m_Desc.Device);

    VkResult ResultVk = vmaMapMemory(LogicDevice.Allocator, m_Allocation, &MapData.Memory);
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, CMapData{});

    return MapData;
}

void CBufferVulkan::UnMap(const CMapData& Data)
{
    const auto& LogicDevice = m_BackendVk->GetGlobalContext().GetLogicalDevice(m_Desc.Device);

    vmaUnmapMemory(LogicDevice.Allocator, m_Allocation);

#if 0
    VkResult Result =  vmaFlushAllocation(LogicDevice.Allocator, m_Allocation, 0, VK_WHOLE_SIZE);
    C_ASSERT_VK_SUCCEEDED(Result);
#endif 
}

} // namespace Cyclone::Render
