#include "RenderBackendDX12.h"

#include "CommonDX12.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"

namespace Cyclone::Render
{

static constexpr int MAX_SUBRESOURCE_COUNT = 10 * 10; // Large enough to support cubemaps with mips

uint32 CalculateSubresourcesCount(const CUploadQueue::CResourceRef& Resource)
{
    // ->GetResource()->GetDesc()
    return 0;
}

C_STATUS RenderBackendDX12::Init(IRenderer* Renderer)
{
    m_Renderer = Renderer;

    C_STATUS Result = C_STATUS::C_STATUS_OK;
    {

    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendDX12::Shutdown()
{
    // #todo_dx12 wait for GPU?
    //m_GlobalContext.Shutdown();

    return C_STATUS::C_STATUS_OK;
}


C_STATUS RenderBackendDX12::BeginRender()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendDX12::EndRender()
{
    return C_STATUS::C_STATUS_OK;
}

CWindowContext* RenderBackendDX12::CreateWindowContext(IWindow* Window)
{
    return nullptr;
}

CCommandQueue* RenderBackendDX12::CreateCommandQueue()
{
    return nullptr;

}

CCommandBuffer* RenderBackendDX12::CreateCommandBuffer()
{
    return nullptr;

}

CResource* RenderBackendDX12::CreateTexture()
{
    return nullptr;
}

void CCommandBufferDX12::CopyBuffer(CCopyBufferDesc Desc)
{
    CCommandBuffer::CopyBuffer(Desc);

#if 1
    CommandBuffer->GetCommandList()->CopyBufferRegion(Entry.dstResource->GetResource(), Entry.dstOffset,
        m_Buffer->GetResource(), Entry.srcOffset, Entry.NumBytes);
#endif
}

void CCommandBufferDX12::CopyBufferToTexture(CCopyBufferDesc Desc)
{
    CCommandBuffer::CopyBufferToTexture(Desc);
#if 1
    D3D12_RESOURCE_DESC resourceDesc = Entry.dstResource->GetResource()->GetDesc();

    if (Desc.SubresourcesCount < 0)
        Desc.SubresourcesCount = CalculateSubresourcesCount(DstResource) - FirstSubresource;

    uint32 firstSubresource = Entry.firstSubresource;
    uint32 subresourcesCount = Entry.subresourcesCount;
    CASSERT(subresourcesCount > 0 && subresourcesCount <= MAX_SUBRESOURCE_COUNT);
    CASSERT(firstSubresource + subresourcesCount <= CalculateSubresourcesCount(resourceDesc));

    D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = device->GetResourceAllocationInfo(0, 1, &resourceDesc);

    uint64 totalSizeInBytes = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT subresourceFootprint[MAX_SUBRESOURCE_COUNT];
    uint32 numRows[MAX_SUBRESOURCE_COUNT];
    uint64 rowSizeInBytes[MAX_SUBRESOURCE_COUNT];
    device->GetCopyableFootprints(&resourceDesc, firstSubresource, subresourcesCount, Entry.srcOffset, subresourceFootprint, numRows, rowSizeInBytes, &totalSizeInBytes);

    CASSERT(totalSizeInBytes == entry.NumBytes);

    for (uint32 i = 0; i < subresourcesCount; ++i)
    {
        CD3DX12_TEXTURE_COPY_LOCATION dst(Entry.dstResource->GetResource(), i + firstSubresource);
        CD3DX12_TEXTURE_COPY_LOCATION src(m_Buffer->GetResource(), subresourceFootprint[i]);
        CommandBuffer->GetCommandList()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
    }
#endif
}

} // namespace Cyclone::Render
