#pragma once

#include "Engine/Utils/RingBuffer.h"

#include "Resource.h"

#include <queue>

namespace Cyclone::Render
{

class CFence;

static const uint32 DefaultUploadAlignment = 256;

template <class T>
using Queue = std::queue<T>;

struct ENGINE_API CUploadQueueDesc
{
    IRendererBackend* Backend = nullptr;
    CCommandQueue* CommandQueue = nullptr;
    CDeviceHandle DeviceHandle;
    uint64 BufferSize = 16 MB; // #todo_vk #todo_upload dynamically create and destroy multiple buffers when overflow
};

class ENGINE_API CUploadQueue
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CUploadQueue);

    CUploadQueue();
    ~CUploadQueue();

    C_STATUS Init(const CUploadQueueDesc& desc);
    void DeInit() noexcept;

    // Enqueue API
    // copies data to upload heap, but don't call any GPU work to copy to destination resource
    C_STATUS QueueUploadData(const void* Data, uint64 NumBytes, CHandle<CResource> DstResource, uint64 DstResourceByteOffset = 0,
        EImageLayoutType DstResourceDesiredLayoutAfterTransfer = EImageLayoutType::Count,
        EImageUsageType DstResourceDesiredUsageAfterTransfer = EImageUsageType::Count,
        uint32 Alignment = DefaultUploadAlignment, uint16 FirstSubresource = 0, int16 SubresourcesCount = -1);
    // enqueue GPU work to copy data to destination resource
    C_STATUS EnqueueGPUWork(CCommandBuffer* CommandBuffer);

protected:
    void ReleaseMemoryUntilFence(uint64 LastCompletedFenceValue);

protected:
    struct CFrameOffsetEntry
    {
        uint8* Data = nullptr;
        uint64 FenceValue = ~0ull;
    };

    struct CCopyEntry
    {
        uint64 SrcOffset = 0;
        uint64 DstOffset = 0;
        uint64 NumBytes = 0;
        EImageLayoutType DstResourceStateAfter = EImageLayoutType::Undefined;
        EImageUsageType DstResourceUsageAfter = EImageUsageType::None;
        CHandle<CResource> DstResource;
        uint16 FirstSubresource = 0;
        int16 SubresourcesCount = -1;
    };

protected:
    // #todo_upload make own command list? and submit when don't have enough memory
    CHandle<CResource> m_Buffer;
    CHandle<CFence> m_Fence;
    CMapData m_BufferMapData;

    RingBuffer m_RingBuffer;

    Queue<CFrameOffsetEntry> m_FrameOffsetQueue;
    Vector<CCopyEntry> m_UploadQueue;

    bool m_ReentranceFlag = false; // to prevent recursion calls with barriers
    CUploadQueueDesc m_Desc;
};

} // namespace Cyclone::Render
