#pragma once

#include "Engine/Render/Common.h"

#include "Shader.h"
#include "Mesh.h"

namespace Cyclone::Render
{

enum class EPipelineFlags
{
    None,
    Count
};

class ENGINE_API CDescriptorSetDesc
{
public:

};

class ENGINE_API CPipelineDesc
{
public:
    PipelineType Type;
    EPipelineFlags Flags;
    PrimitiveTopologyType PrimitiveTopology;
    RasterizerState Rasterizer;
    DepthStencilState DepthStencil;
    BlendState Blend;

    // #todo_vk Multisample, Dynamic state

    CVertexLayoutDescription VertexLayout;

    Ptr<CShader> VertexShader;
    Ptr<CShader> PixelShader;
    Ptr<CShader> ComputeShader;

    CDescriptorSetDesc DescriptorSetDesc;

    CDeviceHandle DeviceHandle;
    IRendererBackend* RendererBackend = nullptr;

#if ENABLE_DEBUG_RENDER_BACKEND
    String Name;
#endif
};

class ENGINE_API CPipeline
{
public:
    DISABLE_COPY_ENABLE_MOVE(CPipeline);

    CPipeline();
    virtual ~CPipeline();

    virtual C_STATUS Init(const CPipelineDesc& Desc);
    virtual void DeInit();

    virtual void* GetBackendDataPtr() = 0;

    const CPipelineDesc& GetDesc() const { return m_Desc; }

public:

protected:
    CPipelineDesc m_Desc;

};

} // namespace Cyclone::Render
