#pragma once

#include "Engine/Render/Common.h"
#include "Engine/Render/Types/Texture.h"

#include "CommonVulkan.h"

namespace Cyclone::Render
{

class CTextureVulkan : public CTexture
{
public:
    CTextureVulkan();
    virtual ~CTextureVulkan();

    virtual C_STATUS Init(const CTextureDesc& Desc) override;
    virtual void DeInit() override;

    virtual void* GetBackendDataPtr() override { return m_ImageVk; }

    VkImage GetImage() const { return m_ImageVk; }

protected:
    VkImage m_ImageVk = VK_NULL_HANDLE;
    VmaAllocation m_Allocation = VK_NULL_HANDLE;
    VmaAllocationInfo m_AllocationInfo{};

    RenderBackendVulkan* m_BackendVk = nullptr;
    // mem ptr? 
    // default views?
};

} // namespace Cyclone::Render
