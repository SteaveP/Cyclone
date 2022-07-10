#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"
#include "Engine/Render/Handle.h"

namespace Cyclone::Render
{

class CResource;
class CResourceDesc;
class CResourceView;
class CResourceViewDesc;
class CPipelineState;
class CPipelineStateDesc;
class CSampler;
class CSamplerDesc;
class CShader;
class CShaderDesc;
class CFence;
class CFenceDesc;
struct CShaderBindingSet;
struct CVertexLayoutDescription;


#define RESOURCE_MANAGER_ADD(Name, Type)                                  \
    virtual CHandle<Type> Create##Name() = 0;                             \
    virtual CHandle<Type> Create##Name(const Type##Desc& Desc) = 0;    \
    virtual void Destroy##Name(CHandle<Type> Handle) = 0;                 \
                                                                          \
    virtual Type* Get##Name(CHandle<Type> Handle) = 0;                    \
    virtual const Type##Desc* Get##Name##Desc(CHandle<Type> Handle) = 0;  \

// #todo_vk_resource how to extend it? - overload by handle type?
class ENGINE_API IResourceManager
{
public:
    IResourceManager();
    virtual ~IResourceManager();

    // #todo_mt #todo_multithreading: should return LockRef<> to resource to hold lock to access to resource for guarantee that other thread doesn't insert something and cause memory movement for current ptr
    RESOURCE_MANAGER_ADD(Resource, CResource);
    RESOURCE_MANAGER_ADD(ResourceView, CResourceView);
    RESOURCE_MANAGER_ADD(PipelineState, CPipelineState);
    RESOURCE_MANAGER_ADD(Shader, CShader);
    RESOURCE_MANAGER_ADD(Sampler, CSampler);
    RESOURCE_MANAGER_ADD(Fence, CFence);

    virtual CHandle<CPipelineState> GetPipelineStateCached(const CPipelineStateDesc& Desc) = 0;
    virtual void ReleasePipelineStateCached(CHandle<CPipelineState> Handle) = 0;

    virtual CHandle<CSampler> GetSamplerCached(const CSamplerDesc& Desc) = 0;
    virtual void ReleaseSamplerCached(const CSamplerDesc& Desc) = 0;
    virtual void ReleaseSamplerCached(CHandle<CSampler> Handle) = 0;

    // #todo_vk_resource #todo_vk_material
    // Material
    // Mesh
    // RTX types?
    //   Acceleration structure

    virtual C_STATUS AddSystemShaderBindingsTo(CShaderBindingSet& Bindings) = 0;
    virtual C_STATUS AddSystemVertexBindingsTo(CVertexLayoutDescription& Layout) = 0;
};

} // namespace Cyclone::Render
