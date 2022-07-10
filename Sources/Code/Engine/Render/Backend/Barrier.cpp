#include "Barrier.h"

#include "Engine/Render/Backend/IRendererBackend.h"
#include "Engine/Render/Backend/IResourceManager.h"
#include "Engine/Render/Backend/Resource.h"

namespace Cyclone::Render
{

CPipelineBarrier CPipelineBarrier::FromTextureAuto(IRendererBackend* Backend, CHandle<CResource> Resource, 
    EImageLayoutType Layout, EImageUsageType UsageHint, bool KeepContent)
{
    // #todo_vk pass subresource range
    CPipelineBarrier Barrier{
        .Action = EPipelineBarrierAction::TextureMemory,
        .Resource = Resource,
        .NewUsage = UsageHint,
        .SubresourceRange = {}
    };

    CResource* ResourcePtr = Backend->GetResourceManager(CDeviceHandle::From(Resource))->GetResource(Resource);
    CASSERT(ResourcePtr);

    CASSERT(ResourcePtr->GetDesc().Flags & EResourceFlags::Texture);

    if (ResourcePtr->GetDesc().Texture.Usage & (EImageUsageType::DepthStencil | EImageUsageType::DepthStencilRead))
    {
        Barrier.SubresourceRange.AspectMask = EImageAspectType::Depth; // #todo_vk_stencil
    }

    bool IsLayoutTransition = ResourcePtr->GetTraceableLayout() != Layout || KeepContent;
    bool IsQueueOwnershipTransfer = false; // #todo_vk_async_compute
    Barrier.SrcPipelineStageMask = EExecutionStageMask::None;
    Barrier.DstPipelineStageMask = EExecutionStageMask::None;

    if (KeepContent)
    {
        CASSERT(ResourcePtr->GetTraceableLayout() != EImageLayoutType::Undefined);
        CASSERT(ResourcePtr->GetTraceableUsageType() != EImageUsageType::None);
        Barrier.OldLayout = ResourcePtr->GetTraceableLayout();
    }
    else
        Barrier.OldLayout = EImageLayoutType::Undefined;
    Barrier.NewLayout = Layout;

    // todo_vk_material EResourceUsageType 
    auto FillStageFromUsage = [](EImageUsageType Usage, EExecutionStageMask& Stage, bool IsSrc)
    {
        if (Usage & (EImageUsageType::TransferSrc | EImageUsageType::TransferDst))
        {
            Stage |= EExecutionStageMask::Transfer;
        }
        if (Usage & (EImageUsageType::ShaderResourceView | EImageUsageType::Sampled | EImageUsageType::Storage | EImageUsageType::DepthStencilRead))
        {
            // #todo_vk here also should be set Vertex shader stage in some cases
            Stage |= EExecutionStageMask::PixelShader | EExecutionStageMask::ComputeShader;
        }
        if (Usage & EImageUsageType::ColorAttachment)
        {
            Stage |= EExecutionStageMask::ColorAttachmentOutput;
        }
        if (Usage & EImageUsageType::DepthStencil)
        {
            Stage |= EExecutionStageMask::DepthStencil;
        }
    };

    // Src Stage
    FillStageFromUsage(ResourcePtr->GetTraceableUsageType(), Barrier.SrcPipelineStageMask, true);
    // Dst Stage
    FillStageFromUsage(UsageHint, Barrier.DstPipelineStageMask, false);

    auto FillAccessFromUSage = [IsLayoutTransition](EImageUsageType Usage, EMemoryAccessMask& Access, bool IsSrc)
    {
        // read states
        if (IsSrc == false)
        {
            if (Usage & (EImageUsageType::TransferSrc | EImageUsageType::TransferDst))
            {
                Access |= EMemoryAccessMask::TransferRead;
            }
            if (Usage & (EImageUsageType::ShaderResourceView | EImageUsageType::Sampled | EImageUsageType::Storage))
            {
                Access |= EMemoryAccessMask::ShaderRead;
            }
            if (Usage & EImageUsageType::ColorAttachment)
            {
                Access |= EMemoryAccessMask::ColorAttachmentRead;
            }
            if (Usage & (EImageUsageType::DepthStencil | EImageUsageType::DepthStencilRead))
            {
                Access |= EMemoryAccessMask::DepthStencilRead;
            }
        }
        // write states
        if (IsSrc || IsLayoutTransition)
        {
            if (Usage & (EImageUsageType::TransferDst | EImageUsageType::TransferSrc)) // #todo_vk is src needed here?
            {
                Access |= EMemoryAccessMask::TransferWrite;
            }
            if (Usage & (EImageUsageType::ShaderResourceView | EImageUsageType::Sampled | EImageUsageType::Storage))
            {
                Access |= EMemoryAccessMask::ShaderWrite;
            }
            if (Usage & EImageUsageType::ColorAttachment)
            {
                Access |= EMemoryAccessMask::ColorAttachmentWrite;
            }
            if (Usage & EImageUsageType::DepthStencil)
            {
                Access |= EMemoryAccessMask::DepthStencilWrite;
            }
        }
    };

    // Src Access (only write states)
    FillAccessFromUSage(ResourcePtr->GetTraceableUsageType(), Barrier.SrcMemoryAccessMask, true);

    // Dst Access (read + might be write if there is layout transition or queue ownership transfer
    FillAccessFromUSage(UsageHint, Barrier.DstMemoryAccessMask, false);

    return Barrier;
}


} // namespace Cyclone::Render
