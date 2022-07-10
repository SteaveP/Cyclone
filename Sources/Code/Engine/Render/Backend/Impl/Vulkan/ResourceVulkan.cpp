#include "ResourceVulkan.h"
#include "RenderBackendVulkan.h"
#include "CommandQueueVulkan.h"

#include "Internal/DeviceManagerVulkan.h"

#include "Engine/Utils/Log.h"

namespace Cyclone::Render
{

#define RESOURCE_TRACE_DELETION 1

CResourceVulkan::CResourceVulkan() = default;
CResourceVulkan::CResourceVulkan(CResourceVulkan&& Other) noexcept : CResource(MoveTemp(Other))
{
    std::swap(m_ImageVk, Other.m_ImageVk);
    std::swap(m_BufferVk, Other.m_BufferVk);
    std::swap(m_Allocation, Other.m_Allocation);
    std::swap(m_AllocationInfo, Other.m_AllocationInfo);
    std::swap(m_BackendVk, Other.m_BackendVk);
}
CResourceVulkan& CResourceVulkan::operator = (CResourceVulkan&& Other) noexcept
{
    if (this != &Other)
    {
        CResource::operator=(MoveTemp(Other));
        std::swap(m_ImageVk, Other.m_ImageVk);
        std::swap(m_BufferVk, Other.m_BufferVk);
        std::swap(m_Allocation, Other.m_Allocation);
        std::swap(m_AllocationInfo, Other.m_AllocationInfo);
        std::swap(m_BackendVk, Other.m_BackendVk);
    }
    return *this;
}
CResourceVulkan::~CResourceVulkan()
{
    DeInitImpl();

    CASSERT(m_ImageVk == nullptr);
    CASSERT(m_Allocation == nullptr);
}

C_STATUS CResourceVulkan::Init(const CResourceDesc& Desc)
{
    C_STATUS Result = CResource::Init(Desc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    m_BackendVk = GET_BACKEND_IMPL(Desc.Backend);

    // #todo_vk also check flag
    if (Desc.ExternalBackendResource)
    {
        if (Desc.Flags & EResourceFlags::Texture)
            m_ImageVk = reinterpret_cast<VkImage>(Desc.ExternalBackendResource);
        else if (Desc.Flags & EResourceFlags::Buffer)
            m_BufferVk = reinterpret_cast<VkBuffer>(Desc.ExternalBackendResource);
        else
        {
            CASSERT(false && "Unimplemented resource type");
        }
    }
    else
    {
        const auto& Device = m_BackendVk->GetDeviceManager().GetDevice(Desc.DeviceHandle);
        
        VmaAllocationCreateInfo AllocInfo{};
        AllocInfo.usage = VMA_MEMORY_USAGE_AUTO; // #todo_vk refactor?

        uint32_t QueueFamilyIndices[] = { Device.GetCommandQueue(CommandQueueType::Graphics)->GetQueueFamilyIndex() };

        if (Desc.Flags & EResourceFlags::Texture)
        {
            VkImageCreateInfo Info{};
            Info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            Info.pNext = nullptr;

            //VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
            //VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            //VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
            //VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;
            //VK_IMAGE_CREATE_ALIAS_BIT; // ?
            Info.flags = 0;
            Info.imageType = ConvertTextureType(m_Desc.Texture.ImageType);
            Info.format = ConvertFormatType(m_Desc.Format);
            Info.extent = { uint32(m_Desc.Texture.Width), m_Desc.Texture.Height, m_Desc.Texture.Depth };
            Info.mipLevels = m_Desc.Texture.MipLevels;
            Info.arrayLayers = m_Desc.Texture.ArrayCount;
            Info.samples = ConvertSamlpeCountType(m_Desc.Texture.SamplesCount);
            Info.tiling = ConvertTilingType(m_Desc.Texture.Tiling);
            Info.usage = ConvertTextureUsageType(m_Desc.Texture.Usage);
            Info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // #todo_vk_async_compute
            Info.initialLayout = ConvertLayoutType(m_Desc.InitialLayout);

            Info.queueFamilyIndexCount = 1;
            Info.pQueueFamilyIndices = QueueFamilyIndices;

            VkResult Result = vmaCreateImage(Device.Allocator, &Info, &AllocInfo, &m_ImageVk, &m_Allocation, &m_AllocationInfo);
            C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
        }
        else if (Desc.Flags & EResourceFlags::Buffer)
        {
            VkBufferCreateInfo Info{};
            Info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            Info.pNext = nullptr;
            Info.flags = 0;
            Info.size = m_Desc.Buffer.ByteSize;
            Info.usage = ConvertBufferUsageType(m_Desc.Buffer.Usage);
            Info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // #todo_vk async compute

            Info.queueFamilyIndexCount = 1;
            Info.pQueueFamilyIndices = QueueFamilyIndices;

            if (Desc.Flags & EResourceFlags::Mappable)
            {
                // Needed to set VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT flag for memory
                AllocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            }

            VkResult Result = vmaCreateBuffer(Device.Allocator, &Info, &AllocInfo, &m_BufferVk, &m_Allocation, &m_AllocationInfo);
            C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
        }
        else
        {
            CASSERT(false && "Unimplemented resource type");
        }
    }

#if ENABLE_DEBUG_RENDER_BACKEND
    if (m_Desc.Name.empty() == false)
    {
        const auto& Device = m_BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);

        if (m_Allocation != VK_NULL_HANDLE)
        {
            vmaSetAllocationName(Device.Allocator, m_Allocation, ("Memory " + m_Desc.Name).c_str());
        }

        if (Desc.Flags & EResourceFlags::Texture)
            SetDebugNameVk(m_Desc.Name, VK_OBJECT_TYPE_IMAGE, (uint64)m_ImageVk, Device);
        else if (Desc.Flags & EResourceFlags::Buffer)
            SetDebugNameVk(m_Desc.Name, VK_OBJECT_TYPE_BUFFER, (uint64)m_BufferVk, Device);
        else
        {
            CASSERT(false && "Unimplemented resource type");
        }
    }
#endif

    return C_STATUS::C_STATUS_OK;
}

void CResourceVulkan::DeInit()
{
    DeInitImpl();
    CResource::DeInit();
}

CMapData CResourceVulkan::Map()
{
    CMapData MapData{};

    const auto& Device = m_BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);

    VkResult ResultVk = vmaMapMemory(Device.Allocator, m_Allocation, &MapData.Memory);
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, CMapData{});

    return MapData;
}

void CResourceVulkan::UnMap(const CMapData& Data)
{
    const auto& Device = m_BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);

    vmaUnmapMemory(Device.Allocator, m_Allocation);

#if 0
    VkResult Result = vmaFlushAllocation(Device.Allocator, m_Allocation, 0, VK_WHOLE_SIZE);
    C_ASSERT_VK_SUCCEEDED(Result);
#endif 
}

void CResourceVulkan::DeInitImpl()
{
    CRenderBackendVulkan* BackendVk = m_BackendVk;
    CDeviceHandle DeviceHandle = m_Desc.DeviceHandle;

    if (m_ImageVk)
    {
        LOG_TRACE("CResourceVulkan::DeInitImpl Called at frame {}", m_BackendVk->GetRenderer()->GetCurrentFrame());
        CASSERT(m_BufferVk == VK_NULL_HANDLE);
        if (m_Desc.ExternalBackendResource == nullptr)
        {
            m_BackendVk->GetDisposalManagerVk(m_Desc.DeviceHandle)->AddDisposable(
                [BackendVk = m_BackendVk, DeviceHandle = m_Desc.DeviceHandle, ImageVk = m_ImageVk, Alloc = m_Allocation]() 
            {
                LOG_TRACE("CResourceVulkan::DeInitImpl Actual deletion at frame {}", BackendVk->GetRenderer()->GetCurrentFrame());

                const auto& Device = BackendVk->GetDeviceManager().GetDevice(DeviceHandle);
                vmaDestroyImage(Device.Allocator, ImageVk, Alloc);
            });
        }
    }

    if (m_BufferVk)
    {
#if RESOURCE_TRACE_DELETION
        LOG_INFO("CResourceVulkan::DeInitImpl ({})", m_BackendVk->GetRenderer()->GetCurrentFrame());
#endif
        CASSERT(m_ImageVk == VK_NULL_HANDLE);
        if (m_Desc.ExternalBackendResource == nullptr)
        {
            m_BackendVk->GetDisposalManagerVk(m_Desc.DeviceHandle)->AddDisposable(
                [BackendVk = m_BackendVk, DeviceHandle = m_Desc.DeviceHandle, BufferVk = m_BufferVk, Alloc = m_Allocation]()
            {
#if RESOURCE_TRACE_DELETION
                LOG_INFO("CResourceVulkan::DeInitImpl Real ({})", BackendVk->GetRenderer()->GetCurrentFrame());
#endif
                const auto& Device = BackendVk->GetDeviceManager().GetDevice(DeviceHandle);
                vmaDestroyBuffer(Device.Allocator, BufferVk, Alloc);
            });
        }
    }

    m_ImageVk = VK_NULL_HANDLE;
    m_BufferVk = VK_NULL_HANDLE;
    m_Allocation = VK_NULL_HANDLE;
}

} // namespace Cyclone::Render
