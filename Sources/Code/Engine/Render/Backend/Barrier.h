#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Render/CommonRender.h"

namespace Cyclone::Render
{
    
class ENGINE_API CPipelineBarrier
{
public:
    EPipelineBarrierAction Action = EPipelineBarrierAction::GlobalMemory;
    
    CHandle<CResource> Resource;
    EImageUsageType NewUsage = EImageUsageType::None; // todo_vk_material EResourceUsageType

    // Texture
    CTextureSubresourceRange SubresourceRange;
    EImageLayoutType OldLayout = EImageLayoutType::Undefined;
    EImageLayoutType NewLayout = EImageLayoutType::Undefined;

    // Buffer
    uint64 Offset = 0;
    uint64 Size = 0;

    // Src
    EExecutionStageMask SrcPipelineStageMask = EExecutionStageMask::None;
    EMemoryAccessMask SrcMemoryAccessMask = EMemoryAccessMask::None;
    // Dst
    EExecutionStageMask DstPipelineStageMask = EExecutionStageMask::None;
    EMemoryAccessMask DstMemoryAccessMask = EMemoryAccessMask::None;

    static CPipelineBarrier FromTextureAuto(IRendererBackend* Backend, CHandle<CResource> Resource, EImageLayoutType Layout, EImageUsageType UsageHint, bool KeepContent = true);

    static CPipelineBarrier FromTexture(CHandle<CResource> Tex,
        EImageLayoutType OldLayout = EImageLayoutType::ColorAttachment,
        EImageLayoutType NewLayout = EImageLayoutType::ReadOnly,
        CTextureSubresourceRange Range = {},
        EExecutionStageMask SrcPipelineStageMask = EExecutionStageMask::None,
        EMemoryAccessMask SrcMemoryAccessMask = EMemoryAccessMask::None,
        EExecutionStageMask DstPipelineStageMask = EExecutionStageMask::None,
        EMemoryAccessMask DstMemoryAccessMask = EMemoryAccessMask::None);
};

inline CPipelineBarrier CPipelineBarrier::FromTexture(CHandle<CResource> Tex, EImageLayoutType OldLayout, EImageLayoutType NewLayout, CTextureSubresourceRange Range, EExecutionStageMask SrcPipelineStageMask, EMemoryAccessMask SrcMemoryAccessMask, EExecutionStageMask DstPipelineStageMask /*= EExecutionStageMask::BottomOfPipe*/, EMemoryAccessMask DstMemoryAccessMask)
{
    return CPipelineBarrier{
        .Action = EPipelineBarrierAction::TextureMemory,
        .Resource = Tex,
        .SubresourceRange = Range,
        .OldLayout = OldLayout,
        .NewLayout = NewLayout,
        .SrcPipelineStageMask = SrcPipelineStageMask,
        .SrcMemoryAccessMask = SrcMemoryAccessMask,
        .DstPipelineStageMask = DstPipelineStageMask,
        .DstMemoryAccessMask = DstMemoryAccessMask,
    };
}


} // namespace Cyclone::Render
