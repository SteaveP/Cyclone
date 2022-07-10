#pragma once

#include "Engine/Render/Backend/ResourceView.h"
#include "CommonVulkan.h"

namespace Cyclone::Render
{

class CResourceViewVulkan : public CResourceView
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CResourceViewVulkan);

    CResourceViewVulkan();
    virtual ~CResourceViewVulkan();

    virtual C_STATUS Init(const CResourceViewDesc& Desc);
    virtual void DeInit();

    VkImageView GetTextureView() const { return m_TextureViewVk; }
    VkBufferView GetBufferView() const { return m_BufferViewVk; }

private:
    void DeInitImpl();

protected:
    VkImageView m_TextureViewVk = VK_NULL_HANDLE;
    VkBufferView m_BufferViewVk = VK_NULL_HANDLE;
};

} // namespace Cyclone::Render
