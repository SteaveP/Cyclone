#pragma once

#include "Engine/Render/CommonRender.h"
#include "Engine/Render/Handle.h"
#include "Engine/Render/Mesh.h"

#include "Shader.h"

namespace Cyclone::Render
{

enum class EPipelineFlags
{
    None,
    Count
};

class ENGINE_API CPipelineStateDesc
{
public:
    PipelineType Type;
    EPipelineFlags Flags;
    PrimitiveTopologyType PrimitiveTopology;
    
    CHandle<CRasterizerState> Rasterizer;
    CHandle<CDepthStencilState> DepthStencil;
    CHandle<CBlendState> Blend;

    // #todo_vk Dynamic state

    CVertexLayoutDescription VertexLayout;

    CHandle<CShader> VertexShader;
    CHandle<CShader> PixelShader;
    CHandle<CShader> ComputeShader;

    CShaderBindingSet ShaderBindingSet;

    // Information about render targets
    Vector<EFormatType> RenderTargets;
    Optional<EFormatType> DepthTarget;
    Optional<EFormatType> StencilTarget;

    CDeviceHandle DeviceHandle;
    IRendererBackend* Backend = nullptr;

#if ENABLE_DEBUG_RENDER_BACKEND
    String Name;
#endif

    bool operator ==(const CPipelineStateDesc& Other) const noexcept = default;
};

class ENGINE_API CPipelineState
{
public:
    DISABLE_COPY_ENABLE_MOVE(CPipelineState);

    CPipelineState();
    virtual ~CPipelineState();

    virtual C_STATUS Init(const CPipelineStateDesc& Desc);
    virtual void DeInit();

    virtual void* GetBackendDataPtr() const = 0;

    const CPipelineStateDesc& GetDesc() const { return m_Desc; }

protected:
    CPipelineStateDesc m_Desc;

};

} // namespace Cyclone::Render
