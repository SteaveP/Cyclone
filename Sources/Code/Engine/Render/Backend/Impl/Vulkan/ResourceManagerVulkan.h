#pragma once

#include "Engine/Render/Backend/IBindlessManager.h"
#include "Engine/Render/Backend/IResourceManager.h"

#include "Engine/Render/Utils/Pool.h"

#include "CommonVulkan.h"
#include "Internal/PipelineStateVulkan.h" // #todo_vk refactor CPipelineHandle and remove this

namespace Cyclone::Render
{

class CFenceVulkan;
class CShaderVulkan;

class CRenderPassHasher
{
public:
    std::size_t operator()(const CRenderPass& Key) const noexcept;
};

class CPipelineStateHasher
{
public:
    std::size_t operator()(const CPipelineStateDesc& Key) const noexcept;
};

class CDescriptorSetLayoutHasher
{
public:
    std::size_t operator()(const CDescriptorSetLayout& Key) const noexcept;
};

class CSamplerDescHasher
{
public:
    std::size_t operator()(const CSamplerDesc& Key) const noexcept;
};

struct CDescriptorPoolVkDesc
{
    uint32 DescriptorsCount = 8 * 1024;
    bool Bindless = false;
    CDeviceHandle DeviceHandle;
    CRenderBackendVulkan* BackendVk = nullptr;
#if ENABLE_DEBUG_RENDER_BACKEND
    String Name;
#endif
};

class CDescriptorPoolVk
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CDescriptorPoolVk);
    CDescriptorPoolVk();
    ~CDescriptorPoolVk();

    C_STATUS Init(const CDescriptorPoolVkDesc& Desc);
    void DeInit();

    VkDescriptorPool Get() const { return m_DescriptorPool; }

protected:
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    CDescriptorPoolVkDesc m_Desc;
};

struct CBindlessDescriptorManagerVulkanDesc
{
    CDescriptorPoolVkDesc PoolDesc;
    CRenderBackendVulkan* BackendVk = nullptr;
    uint64 BindlessBufferSize = 1 MB; // #todo_config read values from config
};

class CBindlessDescriptorManagerVulkan : public IBindlessManager
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CBindlessDescriptorManagerVulkan);

    CBindlessDescriptorManagerVulkan();
    ~CBindlessDescriptorManagerVulkan();

    C_STATUS Init(const CBindlessDescriptorManagerVulkanDesc& Desc);
    void DeInit();

    virtual CBindlessHandle RegisterResource(CHandle<CResourceView> ResourceView) override;
    virtual void UnRegisterResource(CBindlessHandle Handle) override;

    void Update(CBindlessHandle Handle, CHandle<CResourceView> ResourceView);

    virtual CAllocHandle AllocatePersistent(const CAllocDesc& Desc) override;
    virtual void FreePersistent(const CAllocHandle& Handle) override;

    virtual C_STATUS AddSystemShaderBindingsTo(CShaderBindingSet& Bindings);
    virtual C_STATUS AddSystemVertexBindingsTo(CVertexLayoutDescription& VertexLayout);

    void Bind(CCommandBufferVulkan* CommandBufferVk, PipelineType Type);

    CDescriptorPoolVk* Get() { return &m_DescriptorPool; }

protected:
    // #todo_vk should be double buffered
    CDescriptorPoolVk m_DescriptorPool;
    VkDescriptorSet m_TexDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSet m_BufDescriptorSet = VK_NULL_HANDLE; // #todo_vk_resource #todo_vk_material use wrapper class

    CDescriptorSetLayoutVulkan* m_DescrLayoutForTex = VK_NULL_HANDLE;
    CDescriptorSetLayoutVulkan* m_DescrLayoutForBuf = VK_NULL_HANDLE;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

    // alloc
    // mem manager
    uint32 m_TextureCount = 0;
    uint32 m_BufferCount = 0;
    // 

    CBindlessDescriptorManagerVulkanDesc m_Desc;

    // #todo_vk fixme
    CHandle<CResource> m_BindlessGlobalBuffer;
    CHandle<CResourceView> m_BindlessGlobalBufferView;

    uint64 m_BindlessBufferAllocatedByteSize = 0;
    CBindlessHandle m_BindlessBufferBindlessHandle = InvalidBindlessHandle;
};

struct CResourceManagerVulkanDesc
{
    CRenderBackendVulkan* BackendVk = nullptr;
    CDeviceHandle DeviceHandle;
};

// #todo_vk need to support multithreading
class CResourceManagerVulkan : public IResourceManager
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CResourceManagerVulkan);

    CResourceManagerVulkan();
    ~CResourceManagerVulkan();

    C_STATUS Init(const CResourceManagerVulkanDesc& Desc);
    void DeInit();

    //////////////////////////////////////////////////////////////////////////
    // Creation
    //////////////////////////////////////////////////////////////////////////
    
    // Cached
    virtual CHandle<CPipelineState> GetPipelineStateCached(const CPipelineStateDesc& Desc) override;
    virtual void ReleasePipelineStateCached(CHandle<CPipelineState> Handle) override;

    virtual CHandle<CSampler> GetSamplerCached(const CSamplerDesc& Desc) override;
    virtual void ReleaseSamplerCached(const CSamplerDesc& Desc) override;
    virtual void ReleaseSamplerCached(CHandle<CSampler> Handle) override;

    // Uncached
    virtual CHandle<CResource>      CreateResource() override;
    virtual CHandle<CResourceView>  CreateResourceView() override;
    virtual CHandle<CPipelineState> CreatePipelineState() override;
    virtual CHandle<CSampler>       CreateSampler() override;
    virtual CHandle<CShader>        CreateShader() override;
    virtual CHandle<CFence>         CreateFence() override;

    virtual CHandle<CResource>      CreateResource(const CResourceDesc& Desc) override;
    virtual CHandle<CResourceView>  CreateResourceView(const CResourceViewDesc& Desc) override;
    virtual CHandle<CPipelineState> CreatePipelineState(const CPipelineStateDesc& Desc) override;
    virtual CHandle<CSampler>       CreateSampler(const CSamplerDesc& Desc) override;
    virtual CHandle<CShader>        CreateShader(const CShaderDesc& Desc) override;
    virtual CHandle<CFence>         CreateFence(const CFenceDesc& Desc) override;

    // Destruction
    virtual void DestroyResource(CHandle<CResource> Handle) override;
    virtual void DestroyResourceView(CHandle<CResourceView> Handle) override;
    virtual void DestroyPipelineState(CHandle<CPipelineState> Handle) override;
    virtual void DestroySampler(CHandle<CSampler> Handle) override;
    virtual void DestroyShader(CHandle<CShader> Handle) override;
    virtual void DestroyFence(CHandle<CFence> Handle) override;

    // Getters
    virtual CResource*      GetResource(CHandle<CResource> Handle) override;
    virtual CResourceView*  GetResourceView(CHandle<CResourceView> Handle) override;
    virtual CPipelineState* GetPipelineState(CHandle<CPipelineState> Handle) override;
    virtual CSampler*       GetSampler(CHandle<CSampler> Handle) override;
    virtual CShader*        GetShader(CHandle<CShader> Handle) override;
    virtual CFence*         GetFence(CHandle<CFence> Handle) override;

    virtual const CResourceDesc*     GetResourceDesc(CHandle<CResource> Handle) override;
    virtual const CResourceViewDesc* GetResourceViewDesc(CHandle<CResourceView> Handle) override;
    virtual const CPipelineStateDesc*GetPipelineStateDesc(CHandle<CPipelineState> Handle) override;
    virtual const CSamplerDesc*      GetSamplerDesc(CHandle<CSampler> Handle) override;
    virtual const CShaderDesc*       GetShaderDesc(CHandle<CShader> Handle) override;
    virtual const CFenceDesc*        GetFenceDesc(CHandle<CFence> Handle) override;

    //////////////////////////////////////////////////////////////////////////

    virtual C_STATUS AddSystemShaderBindingsTo(CShaderBindingSet& Bindings) override;
    virtual C_STATUS AddSystemVertexBindingsTo(CVertexLayoutDescription& VertexLayout) override;

    CDescriptorSetLayoutVulkan* GetDescriptorSetLayout(const CDescriptorSetLayout& Desc);    
    void ReleaseDescriptorSetLayout(const CDescriptorSetLayout& Desc);

    CRenderBackendVulkan* GetBackendVk() const { return m_Desc.BackendVk; }
    CDescriptorPoolVk* GetDescriptorPool() { return &m_GlobalDescriptorPool; }
    const CDescriptorPoolVk* GetDescriptorPool() const { return &m_GlobalDescriptorPool; }

protected:
    C_STATUS CreateDescriptorPool();

protected:
    CDescriptorPoolVk m_GlobalDescriptorPool;

    // #todo_vk make as flat arrays and return handles to it
    HashMap<CPipelineStateDesc, Pair<uint32, CHandle<CPipelineState>>, CPipelineStateHasher> m_PipelineStatesCache;
    HashMap<CDescriptorSetLayout, Pair<uint32, UniquePtr<CDescriptorSetLayoutVulkan>>, CDescriptorSetLayoutHasher> m_DescriptorSetLayoutCache;
    HashMap<CSamplerDesc, Pair<uint32, CHandle<CSampler>>, CSamplerDescHasher> m_SamplersCache;

    CPool<CResourceVulkan, CResource> m_Resources;
    CPool<CResourceViewVulkan, CResourceView> m_ResourceViews;
    CPool<CPipelineStateVulkan, CPipelineState> m_PipelineStates;
    CPool<CSamplerVulkan, CSampler> m_Samplers;
    CPool<CShaderVulkan, CShader> m_Shaders;
    CPool<CFenceVulkan, CFence> m_Fences;

    CResourceManagerVulkanDesc m_Desc;
};

} // namespace Cyclone::Render
