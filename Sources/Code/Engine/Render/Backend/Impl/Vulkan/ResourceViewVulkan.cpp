#include "ResourceViewVulkan.h"
#include "RenderBackendVulkan.h"
#include "ResourceVulkan.h"
#include "Internal/DeviceManagerVulkan.h"

namespace Cyclone::Render
{

CResourceViewVulkan::CResourceViewVulkan() = default;

CResourceViewVulkan::CResourceViewVulkan(CResourceViewVulkan&& Other) noexcept : CResourceView(MoveTemp(Other))
{
    std::swap(m_TextureViewVk, Other.m_TextureViewVk);
    std::swap(m_BufferViewVk, Other.m_BufferViewVk);
}
CResourceViewVulkan& CResourceViewVulkan::operator = (CResourceViewVulkan&& Other) noexcept
{
    if (this != &Other)
    {
        CResourceView::operator=(MoveTemp(Other));
        std::swap(m_TextureViewVk, Other.m_TextureViewVk);
        std::swap(m_BufferViewVk, Other.m_BufferViewVk);
    }
    return *this;
}
CResourceViewVulkan::~CResourceViewVulkan()
{
    DeInitImpl();
}

C_STATUS CResourceViewVulkan::Init(const CResourceViewDesc& Desc)
{
    C_STATUS Result = CResourceView::Init(Desc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    CRenderBackendVulkan* BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);
    CASSERT(m_Desc.Resource.IsValid());
    CASSERT(BackendVk);

    CDeviceHandle DeviceHandle = CDeviceHandle::From(m_Desc.Resource);
    auto& Device = BackendVk->GetDeviceManager().GetDevice(DeviceHandle);

    CResourceVulkan* ResourceVk = BACKEND_DOWNCAST(Device.ResourceManager->GetResource(m_Desc.Resource), CResourceVulkan);
    C_ASSERT_RETURN_VAL(ResourceVk, C_STATUS::C_STATUS_INVALID_ARG);

    if (Desc.Type & EResourceFlags::Buffer)
    {
        if (ResourceVk->GetDesc().Buffer.Usage & (EBufferUsageType::UniformTexel | EBufferUsageType::StorageTexel))
        {
            CASSERT(ResourceVk->GetDesc().Flags & EResourceFlags::Buffer);

            VkBufferViewCreateInfo CreateInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO };
            CreateInfo.format = ConvertFormatType(m_Desc.Format);
            CreateInfo.offset = m_Desc.Buffer.Offset;
            CreateInfo.range = m_Desc.Buffer.Range;

            CreateInfo.buffer = ResourceVk->GetBuffer();

            VkResult Result = VK_CALL(Device, vkCreateBufferView(Device.DeviceVk, &CreateInfo, nullptr, &m_BufferViewVk));
            C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_INVALID_ARG);

#if ENABLE_DEBUG_RENDER_BACKEND
            if (m_Desc.Name.empty() == false || ResourceVk->GetDesc().Name.empty() == false)
            {
                String Name = m_Desc.Name.empty() == false
                    ? m_Desc.Name
                    : ResourceVk->GetDesc().Name + " BufView";

                SetDebugNameVk(Name, VK_OBJECT_TYPE_BUFFER_VIEW, (uint64)m_BufferViewVk, Device);
            }
#endif
        }
    }
    else if (Desc.Type & EResourceFlags::Texture)
    {
        VkImageViewCreateInfo CreateInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        CreateInfo.image = ResourceVk->GetImage();
        CreateInfo.viewType = ConvertTextureViewType(m_Desc.Texture.ViewType);
        CreateInfo.format = ConvertFormatType(m_Desc.Format);

        CreateInfo.components.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
        CreateInfo.components.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
        CreateInfo.components.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
        CreateInfo.components.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;

        CreateInfo.subresourceRange.aspectMask = ConvertImageAspectType(m_Desc.Texture.AspectMask);
        CreateInfo.subresourceRange.baseMipLevel = m_Desc.Texture.StartMip;
        CreateInfo.subresourceRange.levelCount = m_Desc.Texture.MipLevels == uint16(~0u) ? VK_REMAINING_MIP_LEVELS : m_Desc.Texture.MipLevels;
        CreateInfo.subresourceRange.baseArrayLayer = m_Desc.Texture.StartArrayLayer;
        CreateInfo.subresourceRange.layerCount = m_Desc.Texture.ArrayLayers == uint16(~0u) ? VK_REMAINING_ARRAY_LAYERS : m_Desc.Texture.ArrayLayers;

        VkResult result = VK_CALL(Device, vkCreateImageView(Device.DeviceVk, &CreateInfo, nullptr, &m_TextureViewVk));
        C_ASSERT_VK_SUCCEEDED(result);

#if ENABLE_DEBUG_RENDER_BACKEND
        if (m_Desc.Name.empty() == false || ResourceVk->GetDesc().Name.empty() == false)
        {
            String Name = m_Desc.Name.empty() == false
                ? m_Desc.Name
                : ResourceVk->GetDesc().Name + " TexView";

            SetDebugNameVk(Name, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64)m_TextureViewVk, Device);
        }
#endif
    }
    else
    {
        CASSERT(false);
    }
    
    return C_STATUS::C_STATUS_OK;
}

void CResourceViewVulkan::DeInit()
{
    DeInitImpl();
    CResourceView::DeInit();
}

void CResourceViewVulkan::DeInitImpl()
{
    if (m_BufferViewVk || m_TextureViewVk)
    {
        CRenderBackendVulkan* BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);
        CASSERT(m_Desc.Resource.IsValid());
        CASSERT(BackendVk);

        CDeviceHandle DeviceHandle = CDeviceHandle::From(m_Desc.Resource);

        BackendVk->GetDisposalManagerVk(DeviceHandle)->AddDisposable([Backend = BackendVk, DeviceHandle, 
            Buf  = m_BufferViewVk, Tex = m_TextureViewVk]()
        {
            const auto& Device = Backend->GetDeviceManager().GetDevice(DeviceHandle);

            if (Buf)
                VK_CALL(Device, vkDestroyBufferView(Device.DeviceVk, Buf, nullptr));

            if (Tex)
                VK_CALL(Device, vkDestroyImageView(Device.DeviceVk, Tex, nullptr));
        });

        m_BufferViewVk = VK_NULL_HANDLE;
        m_TextureViewVk = VK_NULL_HANDLE;
    }
}

} // namespace Cyclone::Render
