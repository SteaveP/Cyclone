#pragma once

#include "Engine/Render/Common.h"
#include "Engine/Render/Types/Buffer.h"

#include "CommonVulkan.h"

namespace Cyclone::Render
{

class CBufferVulkan : public CBuffer
{
public:
    CBufferVulkan();
    virtual ~CBufferVulkan();

    virtual C_STATUS Init(const CBufferDesc& Desc) override;
    virtual void DeInit() override;

    virtual CMapData Map() override;
    virtual void UnMap(const CMapData& Data) override;

    virtual void* GetBackendDataPtr() override { return m_BufferVk; }

    VkBuffer GetBuffer() const { return m_BufferVk; }

protected:
    VkBuffer m_BufferVk = VK_NULL_HANDLE;
    VmaAllocation m_Allocation = VK_NULL_HANDLE;
    VmaAllocationInfo m_AllocationInfo{};

    RenderBackendVulkan* m_BackendVk = nullptr;
};

} // namespace Cyclone::Render
