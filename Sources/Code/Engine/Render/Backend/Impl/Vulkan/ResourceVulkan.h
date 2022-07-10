#pragma once

#include "Engine/Render/Backend/Resource.h"

#include "CommonVulkan.h"

namespace Cyclone::Render
{

class CResourceVulkan : public CResource
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CResourceVulkan);

    CResourceVulkan();
    virtual ~CResourceVulkan();

    virtual C_STATUS Init(const CResourceDesc& Desc) override;
    virtual void DeInit() override;

    virtual RawPtr GetBackendDataPtr() override {
        return (m_Desc.Flags & EResourceFlags::Texture) ? (RawPtr)m_ImageVk : (RawPtr)m_BufferVk; }

    virtual CMapData Map() override;
    virtual void UnMap(const CMapData& Data) override;

    VkImage GetImage() const { return m_ImageVk; }
    VkBuffer GetBuffer() const { return m_BufferVk; }

private:
    void DeInitImpl();

protected:
    VkImage m_ImageVk = VK_NULL_HANDLE;
    VkBuffer m_BufferVk = VK_NULL_HANDLE;

    VmaAllocation m_Allocation = VK_NULL_HANDLE;
    VmaAllocationInfo m_AllocationInfo{};

    CRenderBackendVulkan* m_BackendVk = nullptr;
};

} // namespace Cyclone::Render
