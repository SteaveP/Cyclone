#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Framework/IRenderer.h"

#include "Engine/Render/CommonRender.h"

namespace Cyclone::Render
{

class CPipelineBarrier;

struct CCopyBufferDesc
{
    CHandle<CResource> SrcBuffer;
    CHandle<CResource> DstBuffer;
    uint64 SrcOffset = 0;
    uint64 DstOffset = 0;
    uint64 NumBytes = 0;
    uint32 FirstSubresource = 0;
    int32 SubresourcesCount = -1;
};

struct CCopyBufferToTextureDesc
{
    CHandle<CResource> SrcBuffer;
    CHandle<CResource> DstTexture;
    uint64 SrcBaseOffset = 0;
    uint64 DstBaseOffset = 0;
    uint64 NumBytes = 0;
    // #todo_vk #todo_upload
    uint32 FirstSubresource = 0;
    int32 SubresourcesCount = -1;
};


class ENGINE_API CCommandContext
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CCommandContext);

    CCommandContext();
    virtual ~CCommandContext();

    virtual C_STATUS Init(IRendererBackend* Backend, CCommandBuffer* CommandBuffer);
    virtual void DeInit();

    virtual void OnBegin(CCommandBuffer* CommandBuffer);
    virtual void OnEnd(CCommandBuffer* CommandBuffer);

    virtual void BeginRenderPass(CCommandBuffer* CommandBuffer, const CRenderPass& RenderPass);
    virtual void EndRenderPass(CCommandBuffer* CommandBuffer);

    virtual void SetIndexBuffer(CCommandBuffer* CommandBuffer, CHandle<CResource> IndexBuffer, uint64 Offset, EFormatType Format = EFormatType::R16_UINT) = 0;
    virtual void SetVertexBuffer(CCommandBuffer* CommandBuffer, CHandle<CResource> VertexBuffer, uint32 Slot, uint64 Offset) = 0;
    
    virtual void InsertPipelineBarrier(CCommandBuffer* CommandBuffer, const CPipelineBarrier& Barrier, bool ForceFlush);
    virtual void FLushPendingPipelineBarriers(CCommandBuffer* CommandBuffer);

    virtual void SetPipeline(CCommandBuffer* CommandBuffer, CHandle<CPipelineState> Pipeline);

private:
    void DeInitImpl() noexcept;

protected:
    IRendererBackend* m_Backend = nullptr;
    CCommandBuffer* m_CommandBuffer = nullptr;
    CRenderPass m_ActiveRenderPass;

    Vector<CPipelineBarrier> m_PendingBarriers;
};

class ENGINE_API CCommandBuffer
{
public:
    DISABLE_COPY_ENABLE_MOVE(CCommandBuffer);

    CCommandBuffer();
    virtual ~CCommandBuffer();

    virtual void DeInit();

    virtual C_STATUS Begin();
    virtual void End();

    // Set State commands
    virtual void BeginRenderPass(const CRenderPass& RenderPass);
    virtual void EndRenderPass();
 
    virtual void SetPipeline(CHandle<CPipelineState> Pipeline);
    virtual void SetRenderTargets(CRenderTargetSet RenderTargetSet);

    virtual void SetIndexBuffer(CHandle<CResource> IndexBuffer, uint64 Offset, EFormatType Format = EFormatType::R16_UINT);
    virtual void SetVertexBuffer(CHandle<CResource> VertexBuffer, uint32 Slot, uint64 Offset);

    // #todo_vk array versions
    virtual void InsertPipelineBarrier(CPipelineBarrier Barrier, bool ForceFlush = false);
    virtual void FLushPendingPipelineBarriers();

    // Draw commands
    virtual void Draw(uint32 IndexCount, uint32 InstanceCount, uint32 FirstIndex, int32 VertexOffset, uint32 FirstInstance);

    // Dispatch commands
    virtual void Dispatch();

    // Copy commands
    virtual void CopyBuffer(const CCopyBufferDesc& Desc);
    virtual void CopyBufferToTexture(const CCopyBufferToTextureDesc& Desc);

    // Clear commands
    virtual void Clear();

    // Indirect commands
    // #todo_vk

    CCommandQueue* GetCommandQueue() { return m_CommandQueue; }
    const CCommandQueue* GetCommandQueue() const { return m_CommandQueue; }
    IRendererBackend* GetBackend() { return m_Backend; }
    const IRendererBackend* GetBackend() const { return m_Backend; }

    CDeviceHandle GetDeviceHandle() const { return m_DeviceHandle; }

private:
    void DeInitImpl() noexcept;

protected:
    CCommandQueue* m_CommandQueue = nullptr;
    IRendererBackend* m_Backend = nullptr;
    CDeviceHandle m_DeviceHandle;
    UniquePtr<CCommandContext> m_Context;
};

} // namespace Cyclone::Render
