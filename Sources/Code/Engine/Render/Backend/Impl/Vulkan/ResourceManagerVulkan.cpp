#include "ResourceManagerVulkan.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"

#include "ResourceVulkan.h"
#include "ResourceViewVulkan.h"
#include "FenceVulkan.h"
#include "ShaderVulkan.h"

#include "RenderBackendVulkan.h"

#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"

namespace Cyclone::Render
{

#define HASH(Type, V) std::hash<Type>()(V)

CResourceManagerVulkan::CResourceManagerVulkan() = default;
CResourceManagerVulkan::CResourceManagerVulkan(CResourceManagerVulkan&& Other) noexcept : IResourceManager(MoveTemp(Other))
{
    std::swap(m_PipelineStatesCache, Other.m_PipelineStatesCache);
    std::swap(m_DescriptorSetLayoutCache, Other.m_DescriptorSetLayoutCache);
    std::swap(m_SamplersCache, Other.m_SamplersCache);
    std::swap(m_GlobalDescriptorPool, Other.m_GlobalDescriptorPool);
    std::swap(m_Desc, Other.m_Desc);
    std::swap(m_Resources, Other.m_Resources);
    std::swap(m_ResourceViews, Other.m_ResourceViews);
    std::swap(m_Shaders, Other.m_Shaders);
}
CResourceManagerVulkan& CResourceManagerVulkan::operator=(CResourceManagerVulkan&& Other) noexcept
{
    if (this != &Other)
    {
        IResourceManager::operator=(MoveTemp(Other));
        std::swap(m_PipelineStatesCache, Other.m_PipelineStatesCache);
        std::swap(m_DescriptorSetLayoutCache, Other.m_DescriptorSetLayoutCache);
        std::swap(m_SamplersCache, Other.m_SamplersCache);
        std::swap(m_GlobalDescriptorPool, Other.m_GlobalDescriptorPool);
        std::swap(m_Desc, Other.m_Desc);
        std::swap(m_Resources, Other.m_Resources);
        std::swap(m_ResourceViews, Other.m_ResourceViews);
        std::swap(m_Shaders, Other.m_Shaders);
    }
    return *this;
}

CResourceManagerVulkan::~CResourceManagerVulkan()
{
    DeInit();

    CASSERT(m_GlobalDescriptorPool.Get() == VK_NULL_HANDLE);
}

C_STATUS CResourceManagerVulkan::Init(const CResourceManagerVulkanDesc& Desc)
{
    m_Desc = Desc;

    C_STATUS Result = CreateDescriptorPool();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

void CResourceManagerVulkan::DeInit()
{
    m_GlobalDescriptorPool.DeInit();

    CASSERT(m_DescriptorSetLayoutCache.empty());
    CASSERT(m_SamplersCache.empty());
    CASSERT(m_PipelineStatesCache.empty());

    m_PipelineStatesCache.clear();
    m_DescriptorSetLayoutCache.clear();
    m_SamplersCache.clear();

    m_ResourceViews = {};
    m_Resources = {};
    m_Shaders = {};
    m_Fences = {};
}

C_STATUS CResourceManagerVulkan::CreateDescriptorPool()
{
    CDescriptorPoolVkDesc Desc{};
    Desc.DescriptorsCount = 16 KB; // #todo_config read values from config
    Desc.DeviceHandle = m_Desc.DeviceHandle;
    Desc.BackendVk = GetBackendVk();
#if ENABLE_DEBUG_RENDER_BACKEND
    Desc.Name = "DescriptorPoolGlobal";
#endif
    C_STATUS Result = m_GlobalDescriptorPool.Init(Desc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

CDescriptorSetLayoutVulkan* CResourceManagerVulkan::GetDescriptorSetLayout(const CDescriptorSetLayout& Desc)
{
    auto It = m_DescriptorSetLayoutCache.find(Desc);
    if (It != m_DescriptorSetLayoutCache.end())
        return It->second.second.get();

    auto& Data = m_DescriptorSetLayoutCache[Desc];
    Data.first = 1;
    auto& DescriptorSetLayoutVkPtr = Data.second;
    DescriptorSetLayoutVkPtr = MakeUnique<CDescriptorSetLayoutVulkan>();

    C_STATUS Result = DescriptorSetLayoutVkPtr->Init(GetBackendVk(), m_Desc.DeviceHandle, Desc);
    CASSERT(C_SUCCEEDED(Result));

    return DescriptorSetLayoutVkPtr.get();
}

std::size_t CRenderPassHasher::operator()(const CRenderPass& Key) const noexcept
{
    std::size_t Value = HASH(uint32, Key.RenderTargetSet.RenderTargetsCount) << 32;

    auto MakeHash = [&](const auto& Slot, uint32 i)
    {
        std::size_t Result = 0;
        Result ^= (CHandleHasher()(Slot.RenderTarget.Texture) >> (5 + i));
        Result ^= (CHandleHasher()(Slot.RenderTarget.RenderTargetView) >> (1 + i));
        Result ^= (CHandleHasher()(Slot.RenderTarget.DepthStencilView) >> (0 + i));
        Result ^= (CHandleHasher()(Slot.RenderTarget.ShaderResourceView) >> (3 + i));
        Result ^= (HASH(uint32, (uint32)Slot.FinalLayout) >> (10 + i));
        Result ^= (HASH(uint32, (uint32)Slot.FinalUsage) >> (15 + i));
        Result ^= (HASH(uint32, (uint32)Slot.LoadOp) << (16 + i));
        Result ^= (HASH(uint32, (uint32)Slot.StoreOp) << (7 + i));
        Result ^= (HASH(uint32, (uint32)Slot.StencilLoadOp) >> (16 + i));
        Result ^= (HASH(uint32, (uint32)Slot.StencilStoreOp) >> (7 + i));
        return Result;
    };

    for (uint32 i = 0; i < Key.RenderTargetSet.RenderTargetsCount; ++i)
    {
        Value ^= MakeHash(Key.RenderTargetSet.RenderTargets[i], i);
    }

    Value ^= MakeHash(Key.RenderTargetSet.DepthScentil, 8);

    return Value;
}

std::size_t CPipelineStateHasher::operator()(const CPipelineStateDesc& Key) const noexcept
{
    std::size_t Value = 0;

    for (uint32 i = 0; i < Key.ShaderBindingSet.SetLayouts.size(); ++i)
    {
        Value ^= CDescriptorSetLayoutHasher()(Key.ShaderBindingSet.SetLayouts[i]) >> (i);
    }

    Value ^= (HASH(uint32, (uint32)Key.Type));
    Value ^= (HASH(uint32, (uint32)Key.Flags));
    Value ^= (HASH(uint32, (uint32)Key.PrimitiveTopology));
    Value ^= (CHandleHasher()(Key.Rasterizer) << 1);
    Value ^= (CHandleHasher()(Key.DepthStencil) << 6);
    Value ^= (CHandleHasher()(Key.Blend) << 2);

    Value ^= (CHandleHasher()(Key.VertexShader) << 3);
    Value ^= (CHandleHasher()(Key.PixelShader));
    Value ^= (CHandleHasher()(Key.ComputeShader) >> 1);

    Value ^= (HASH(uint32, (uint32)Key.DeviceHandle.DeviceHandle) << (16));
    Value ^= (HASH(uint32, (uint32)Key.DeviceHandle.PhysDeviceHandle) << (24));
    Value ^= (HASH(uint64, (uint64)Key.Backend));

    Value ^= (HASH(uint32, (uint32)Key.RenderTargets.size()) << (3));
    Value ^= (HASH(uint32, (uint32)Key.DepthTarget.value_or(EFormatType::Undefined)) << (5));
    Value ^= (HASH(uint32, (uint32)Key.StencilTarget.value_or(EFormatType::Undefined)) << (6));

    for (uint32 i = 0; i < (uint32)Key.RenderTargets.size(); ++i)
    {
        Value ^= (HASH(uint32, (uint32)Key.RenderTargets[i]) << (i));
    }

    return Value;
}

std::size_t CDescriptorSetLayoutHasher::operator()(const CDescriptorSetLayout& Key) const noexcept
{
    std::size_t Value = (HASH(uint32, (uint32)Key.Flags) << 10);

    for (uint32 i = 0; i < Key.Bindings.size(); ++i)
    {
        Value ^= (HASH(uint32, Key.Bindings[i].BindingIndex) >> i);
        Value ^= (HASH(uint32, Key.Bindings[i].DescriptorCount) >> (i + 1));
        Value ^= (HASH(uint32, (uint32)Key.Bindings[i].DescriptorType) << (i + 2));
        Value ^= (HASH(uint32, Key.Bindings[i].StageFlags) >> (i + 3));
        Value ^= (HASH(uint32, (uint32)Key.Bindings[i].Flags) >> (i + 3));
    }

    return Value;
}

std::size_t CSamplerDescHasher::operator()(const CSamplerDesc& Key) const noexcept
{
    // #todo_vk_sampler fixme
    return 0;
}

CDescriptorPoolVk::CDescriptorPoolVk() = default;
CDescriptorPoolVk::CDescriptorPoolVk(CDescriptorPoolVk&& Other) noexcept
{
    std::swap(m_DescriptorPool, Other.m_DescriptorPool);
    std::swap(m_Desc, Other.m_Desc);
}
CDescriptorPoolVk& CDescriptorPoolVk::operator=(CDescriptorPoolVk&& Other) noexcept
{
    if (this != &Other)
    {
        std::swap(m_DescriptorPool, Other.m_DescriptorPool);
        std::swap(m_Desc, Other.m_Desc);
    }
    return *this;
}
CDescriptorPoolVk::~CDescriptorPoolVk()
{
    DeInit();
}

C_STATUS CDescriptorPoolVk::Init(const CDescriptorPoolVkDesc& Desc)
{
    CASSERT(m_DescriptorPool == VK_NULL_HANDLE);

    m_Desc = Desc;

    // #todo_vk #todo_vk_material
    VkDescriptorPoolSize Pools[10]{};
    uint32 PoolCount = 3;

    Pools[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    Pools[0].descriptorCount = Desc.DescriptorsCount;

    Pools[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    Pools[1].descriptorCount = Desc.DescriptorsCount;

    Pools[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    Pools[2].descriptorCount = Desc.DescriptorsCount;

    VkDescriptorPoolCreateInfo DescPoolInfo{};
    DescPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    DescPoolInfo.maxSets = Desc.DescriptorsCount * PoolCount;

    DescPoolInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // #todo_vk #todo_vk_material make this optional
    if (Desc.Bindless)
    {
        DescPoolInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    }
    //DescPoolInfo.flags = VkDescriptorPoolCreateFlagBits:: ;
    DescPoolInfo.poolSizeCount = PoolCount;
    DescPoolInfo.pPoolSizes = Pools;

    auto& Device = m_Desc.BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);
    VkResult ResultVk = VK_CALL(Device, vkCreateDescriptorPool(Device.DeviceVk, &DescPoolInfo, nullptr, &m_DescriptorPool));
    C_ASSERT_VK_SUCCEEDED(ResultVk);

#if ENABLE_DEBUG_RENDER_BACKEND
    if (m_Desc.Name.empty() == false)
        SetDebugNameVk(m_Desc.Name, VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64)m_DescriptorPool, Device);
#endif
    return C_STATUS::C_STATUS_OK;
}

void CDescriptorPoolVk::DeInit()
{
    if (m_DescriptorPool != VK_NULL_HANDLE)
    {
        m_Desc.BackendVk->GetDisposalManagerVk(m_Desc.DeviceHandle)->AddDisposable([Backend = m_Desc.BackendVk, DeviceHandle = m_Desc.DeviceHandle,
            Pool = m_DescriptorPool]()
        {
            const auto& Device = Backend->GetDeviceManager().GetDevice(DeviceHandle);
            VK_CALL(Device, vkDestroyDescriptorPool(Device.DeviceVk, Pool, nullptr));
        });

        m_DescriptorPool = VK_NULL_HANDLE;
    }
}


CBindlessDescriptorManagerVulkan::CBindlessDescriptorManagerVulkan() = default;
CBindlessDescriptorManagerVulkan::CBindlessDescriptorManagerVulkan(CBindlessDescriptorManagerVulkan&& Other) noexcept : IBindlessManager(MoveTemp(Other))
{
    std::swap(m_DescriptorPool, Other.m_DescriptorPool);
    std::swap(m_TexDescriptorSet, Other.m_TexDescriptorSet);
    std::swap(m_BufDescriptorSet, Other.m_BufDescriptorSet);
    std::swap(m_DescrLayoutForTex, Other.m_DescrLayoutForTex);
    std::swap(m_DescrLayoutForBuf, Other.m_DescrLayoutForBuf);
    std::swap(m_PipelineLayout, Other.m_PipelineLayout);
    std::swap(m_TextureCount, Other.m_TextureCount);
    std::swap(m_BufferCount, Other.m_BufferCount);
    std::swap(m_Desc.BackendVk, Other.m_Desc.BackendVk);
    std::swap(m_Desc, Other.m_Desc);
    std::swap(m_BindlessGlobalBuffer, Other.m_BindlessGlobalBuffer);
    std::swap(m_BindlessGlobalBufferView, Other.m_BindlessGlobalBufferView);
    std::swap(m_BindlessBufferAllocatedByteSize, Other.m_BindlessBufferAllocatedByteSize);
    std::swap(m_BindlessBufferBindlessHandle, Other.m_BindlessBufferBindlessHandle);
}
CBindlessDescriptorManagerVulkan& CBindlessDescriptorManagerVulkan::operator=(CBindlessDescriptorManagerVulkan&& Other) noexcept
{
    if (this != &Other)
    {
        IBindlessManager::operator=(MoveTemp(Other));
        std::swap(m_DescriptorPool, Other.m_DescriptorPool);
        std::swap(m_TexDescriptorSet, Other.m_TexDescriptorSet);
        std::swap(m_BufDescriptorSet, Other.m_BufDescriptorSet);
        std::swap(m_DescrLayoutForTex, Other.m_DescrLayoutForTex);
        std::swap(m_DescrLayoutForBuf, Other.m_DescrLayoutForBuf);
        std::swap(m_PipelineLayout, Other.m_PipelineLayout);
        std::swap(m_TextureCount, Other.m_TextureCount);
        std::swap(m_BufferCount, Other.m_BufferCount);
        std::swap(m_Desc.BackendVk, Other.m_Desc.BackendVk);
        std::swap(m_Desc, Other.m_Desc);
        std::swap(m_BindlessGlobalBuffer, Other.m_BindlessGlobalBuffer);
        std::swap(m_BindlessGlobalBufferView, Other.m_BindlessGlobalBufferView);
        std::swap(m_BindlessBufferAllocatedByteSize, Other.m_BindlessBufferAllocatedByteSize);
        std::swap(m_BindlessBufferBindlessHandle, Other.m_BindlessBufferBindlessHandle);
    }
    return *this;
}
CBindlessDescriptorManagerVulkan::~CBindlessDescriptorManagerVulkan()
{
    DeInit();
}

C_STATUS CBindlessDescriptorManagerVulkan::Init(const CBindlessDescriptorManagerVulkanDesc& Desc)
{
    m_Desc = Desc;

    auto& Device = m_Desc.BackendVk->GetDeviceManager().GetDevice(m_Desc.PoolDesc.DeviceHandle);

    IResourceManager* ResourceManager = Device.ResourceManager.get();
    CASSERT(ResourceManager);

    {
        CShaderBindingSet Bindings;
        AddSystemShaderBindingsTo(Bindings);

        CResourceManagerVulkan* ResourceManager = m_Desc.BackendVk->GetResourceManagerVk(m_Desc.PoolDesc.DeviceHandle);
        m_DescrLayoutForBuf = ResourceManager->GetDescriptorSetLayout(Bindings.SetLayouts[0]);
        m_DescrLayoutForTex = ResourceManager->GetDescriptorSetLayout(Bindings.SetLayouts[1]);

        // Pipeline Layout //#todo_vk cache it? or it needs to be removed completely from here
        {
            VkPipelineLayoutCreateInfo LayoutInfo{};
            VkDescriptorSetLayout Layouts[2]{ m_DescrLayoutForBuf->Get(), m_DescrLayoutForTex->Get() };
            LayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            LayoutInfo.setLayoutCount = 2;
            LayoutInfo.pSetLayouts = Layouts;

            VkResult Result = VK_CALL(Device, vkCreatePipelineLayout(Device.DeviceVk, &LayoutInfo, nullptr, &m_PipelineLayout));
#if ENABLE_DEBUG_RENDER_BACKEND
            SetDebugNameVk("PipelineLayoutBindless", VK_OBJECT_TYPE_PIPELINE_LAYOUT, (uint64)m_PipelineLayout, Device);
#endif
            C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
        }
    }
    CASSERT(Desc.PoolDesc.Bindless && Desc.PoolDesc.DescriptorsCount > 1);

    m_BufferCount = m_TextureCount = 0;

    C_STATUS Result = m_DescriptorPool.Init(Desc.PoolDesc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    auto Alloc = [&](auto& Layout, auto& DescriptorSet)
    {
        VkDescriptorSetAllocateInfo AllocInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        VkDescriptorSetLayout LayoutVk = Layout->Get();
        AllocInfo.pSetLayouts = &LayoutVk;
        AllocInfo.descriptorSetCount = 1;
        AllocInfo.descriptorPool = m_DescriptorPool.Get();

        VkDescriptorSetVariableDescriptorCountAllocateInfo CountInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO };
        uint32 MaxBinding = m_Desc.PoolDesc.DescriptorsCount - 1;
        if (m_Desc.PoolDesc.Bindless)
        {
            CountInfo.descriptorSetCount = 1;
            CountInfo.pDescriptorCounts = &MaxBinding;
            AllocInfo.pNext = &CountInfo;
        }

        VkResult ResultVk = VK_CALL(Device, vkAllocateDescriptorSets(Device.DeviceVk, &AllocInfo, &DescriptorSet));
        C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);

        return C_STATUS::C_STATUS_OK;
    };

    Result = Alloc(m_DescrLayoutForTex, m_TexDescriptorSet);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    Result = Alloc(m_DescrLayoutForBuf, m_BufDescriptorSet);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    {
        m_BindlessBufferAllocatedByteSize = 0;
        CResourceDesc BufDesc{};
        BufDesc.Backend = m_Desc.BackendVk;
        BufDesc.DeviceHandle = m_Desc.PoolDesc.DeviceHandle;
        BufDesc.Flags = EResourceFlags::Buffer | EResourceFlags::Mappable; // #todo_vk shouldn't be mappable at least for persistent data
        BufDesc.Buffer.Usage = EBufferUsageType::Index | EBufferUsageType::Indirect | EBufferUsageType::Storage
            | EBufferUsageType::TransferDst | EBufferUsageType::TransferSrc | EBufferUsageType::Uniform | EBufferUsageType::Vertex;
        BufDesc.Buffer.ByteSize = m_Desc.BindlessBufferSize;
#if ENABLE_DEBUG_RENDER_BACKEND
        BufDesc.Name = "BindlessBuffer";
#endif

        m_BindlessGlobalBuffer = ResourceManager->CreateResource(BufDesc);
        C_ASSERT_RETURN_VAL(m_BindlessGlobalBuffer.IsValid(), C_STATUS::C_STATUS_ERROR);

        CResourceViewDesc BufViewDesc{};
        BufViewDesc.Backend = m_Desc.BackendVk;
        BufViewDesc.Type = EResourceFlags::Buffer;
        BufViewDesc.Resource = m_BindlessGlobalBuffer;
        BufViewDesc.Buffer.Offset = 0;
        BufViewDesc.Buffer.Range = static_cast<uint32>(m_Desc.BindlessBufferSize);

        m_BindlessGlobalBufferView = ResourceManager->CreateResourceView(BufViewDesc);
        C_ASSERT_RETURN_VAL(m_BindlessGlobalBufferView.IsValid(), C_STATUS::C_STATUS_ERROR);

        m_BindlessBufferBindlessHandle = RegisterResource(m_BindlessGlobalBufferView);
        CASSERT(m_BindlessBufferBindlessHandle == 0); // #todo_vk fixme should be always 0 or pass this value to the shaders
    }

    return C_STATUS::C_STATUS_OK;
}

void CBindlessDescriptorManagerVulkan::DeInit()
{
    if (m_Desc.BackendVk == nullptr)
        return;

    CRenderBackendVulkan* BackendVk = m_Desc.BackendVk;
    CDeviceHandle DevHandle = m_Desc.PoolDesc.DeviceHandle;

    if (m_BindlessBufferBindlessHandle != InvalidBindlessHandle)
    {
        CASSERT(m_BufDescriptorSet != VK_NULL_HANDLE);
        UnRegisterResource(m_BindlessBufferBindlessHandle);
        m_BindlessBufferBindlessHandle = InvalidBindlessHandle;
    }

    if (m_BindlessGlobalBufferView.IsValid())
    {
        IResourceManager* ResourceManager = BackendVk->GetResourceManager(DevHandle);
        CASSERT(ResourceManager);

        ResourceManager->DestroyResourceView(m_BindlessGlobalBufferView);
        m_BindlessGlobalBufferView = CHandle<CResourceView>{};
    }

    if (m_BindlessGlobalBuffer.IsValid())
    {
        IResourceManager* ResourceManager = BackendVk->GetResourceManager(DevHandle);
        CASSERT(ResourceManager);

        ResourceManager->DestroyResource(m_BindlessGlobalBuffer);
        m_BindlessGlobalBuffer = CHandle<CResource>{};
    }

    if (m_DescrLayoutForTex != nullptr)
    {
        CResourceManagerVulkan* ResourceManager = BackendVk->GetResourceManagerVk(DevHandle);
        ResourceManager->ReleaseDescriptorSetLayout(m_DescrLayoutForTex->GetDesc());
        m_DescrLayoutForTex = nullptr;
    }

    if (m_DescrLayoutForBuf != nullptr)
    {
        CResourceManagerVulkan* ResourceManager = BackendVk->GetResourceManagerVk(DevHandle);
        ResourceManager->ReleaseDescriptorSetLayout(m_DescrLayoutForBuf->GetDesc());
        m_DescrLayoutForBuf = nullptr;
    }

    BackendVk->GetDisposalManagerVk(DevHandle)->AddDisposable([BackendVk, DevHandle, Pool = m_DescriptorPool.Get(),
        Layout = m_PipelineLayout, TexSet = m_TexDescriptorSet, BufSet = m_BufDescriptorSet]()
    {
        auto& Device = BackendVk->GetDeviceManager().GetDevice(DevHandle);

        if (TexSet != VK_NULL_HANDLE)
            VK_CALL(Device, vkFreeDescriptorSets(Device.DeviceVk, Pool, 1, &TexSet));

        if (BufSet!= VK_NULL_HANDLE)
            VK_CALL(Device, vkFreeDescriptorSets(Device.DeviceVk, Pool, 1, &BufSet));

        if (Layout != VK_NULL_HANDLE)
            VK_CALL(Device, vkDestroyPipelineLayout(Device.DeviceVk, Layout, nullptr));
    });

    m_TexDescriptorSet = VK_NULL_HANDLE;
    m_BufDescriptorSet = VK_NULL_HANDLE;
    m_PipelineLayout = VK_NULL_HANDLE;

    m_DescriptorPool.DeInit();
    m_Desc.BackendVk = nullptr;
}

CBindlessHandle CBindlessDescriptorManagerVulkan::RegisterResource(CHandle<CResourceView> ResourceView)
{
    C_ASSERT_RETURN_VAL(ResourceView.IsValid(), CBindlessHandle{});

    IResourceManager* ResourceManager = m_Desc.BackendVk->GetResourceManager(CDeviceHandle::From(ResourceView));
    CASSERT(ResourceManager);

    CResourceView* ResourceViewPtr = ResourceManager->GetResourceView(ResourceView);
    CASSERT(ResourceViewPtr);

    CBindlessHandle Handle;
    if (ResourceViewPtr->GetDesc().Type & EResourceFlags::Buffer)
        Handle = m_BufferCount++;
    else if (ResourceViewPtr->GetDesc().Type & EResourceFlags::Texture)
        Handle = m_TextureCount++;
    else
    {
        CASSERT(false);
    }

    CASSERT(Handle < m_Desc.PoolDesc.DescriptorsCount);

    Update(Handle, ResourceView);

    return Handle;
}

void CBindlessDescriptorManagerVulkan::Bind(CCommandBufferVulkan* CommandBufferVk, PipelineType Type)
{
    VkDescriptorSet Sets[] = { m_BufDescriptorSet, m_TexDescriptorSet };

    auto& Device = m_Desc.BackendVk->GetDeviceManager().GetDevice(CommandBufferVk->GetDeviceHandle());
    VK_CALL(Device, vkCmdBindDescriptorSets(CommandBufferVk->Get(), ConvertPipelineType(Type),
        m_PipelineLayout, 0, 2, Sets, 0, nullptr));
}

void CBindlessDescriptorManagerVulkan::Update(CBindlessHandle Handle, CHandle<CResourceView> ResourceView)
{
    CASSERT(Handle <= m_TextureCount);
    CASSERT(ResourceView.IsValid());

    auto& Device = m_Desc.BackendVk->GetDeviceManager().GetDevice(m_Desc.PoolDesc.DeviceHandle);

    IResourceManager* ResourceManager = Device.ResourceManager.get();
    CASSERT(ResourceManager);

    CResourceViewVulkan* ResourceViewPtr = BACKEND_DOWNCAST(ResourceManager->GetResourceView(ResourceView), CResourceViewVulkan);
    CASSERT(ResourceViewPtr);
    CResourceVulkan* ResourcePtr = BACKEND_DOWNCAST(ResourceManager->GetResource(ResourceViewPtr->GetResource()), CResourceVulkan);
    CASSERT(ResourcePtr);

    VkWriteDescriptorSet Writes[1]{};
    VkDescriptorImageInfo ImageInfo{};
    VkDescriptorBufferInfo BufferInfo{};

    auto& Write = Writes[0];
    Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Write.descriptorCount = 1;
    Write.dstArrayElement = Handle;

    if (ResourcePtr->GetDesc().Flags & EResourceFlags::Buffer)
    {
        Write.dstSet = m_BufDescriptorSet;
        Write.dstBinding = 0;
        Write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

        BufferInfo.buffer = ResourcePtr->GetBuffer();
        BufferInfo.offset = ResourceViewPtr->GetDesc().Buffer.Offset;
        BufferInfo.range = ResourceViewPtr->GetDesc().Buffer.Range;

        Write.pBufferInfo = &BufferInfo;
    }
    else if (ResourcePtr->GetDesc().Flags & EResourceFlags::Texture)
    {
        Write.dstSet = m_TexDescriptorSet;
        Write.dstBinding = 1;
        Write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

        ImageInfo.imageView = ResourceViewPtr->GetTextureView();
        ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL; // #todo_vk_bindless

        Write.pImageInfo = &ImageInfo;
    }

    VK_CALL(Device, vkUpdateDescriptorSets(Device.DeviceVk, 1, Writes, 0, nullptr));
}

CAllocHandle CBindlessDescriptorManagerVulkan::AllocatePersistent(const CAllocDesc& Desc)
{
    static constexpr uint32 Alignment = 4; // #todo_vk fixme
    //CASSERT(Desc.ByteCount % Alignment == 0);
    //CASSERT(Desc.Alignment % Alignment == 0);
    CASSERT(Desc.Alignment == 0); // #todo_vk fixme

    uint64 AlignedByteCount = ((Desc.ByteCount) + Alignment - 1) / Alignment * Alignment;

    CAllocHandle Handle{ .Buffer = m_BindlessGlobalBuffer };
    Handle.DeviceMemBufferDWORDOffset = m_BindlessBufferAllocatedByteSize / Alignment;
    Handle.DeviceMemBufferDWORDCount = AlignedByteCount / Alignment;
    Handle.HostBufferByteOffset = m_BindlessBufferAllocatedByteSize;

    m_BindlessBufferAllocatedByteSize += AlignedByteCount;
    CASSERT(m_BindlessBufferAllocatedByteSize <= m_Desc.BindlessBufferSize);

    return Handle;
}

void CBindlessDescriptorManagerVulkan::FreePersistent(const CAllocHandle& Handle)
{
    // #todo_vk_bindless should be deferred as well?
}

void CBindlessDescriptorManagerVulkan::UnRegisterResource(CBindlessHandle Handle)
{
    // #todo_vk_bindless should be deferred as well?
}

C_STATUS CBindlessDescriptorManagerVulkan::AddSystemShaderBindingsTo(CShaderBindingSet& Bindings)
{
    {
        auto& BufferLayout = Bindings.SetLayouts.emplace_back();
        BufferLayout.Flags |= EDescriptorSetLayoutFlags::Bindless;
        BufferLayout.Bindings.resize(1);
        BufferLayout.Bindings[0].BindingIndex = 0;
        BufferLayout.Bindings[0].DescriptorCount = m_Desc.PoolDesc.DescriptorsCount - 1;
        BufferLayout.Bindings[0].DescriptorType = EDescriptorType::BufferUAV;
        BufferLayout.Bindings[0].Flags |= EDescriptorSetLayoutFlags::Bindless;
    }

    {
        auto& TextureLayout = Bindings.SetLayouts.emplace_back();
        TextureLayout.Bindings.resize(2);
        TextureLayout.Bindings[0].BindingIndex = 0;
        TextureLayout.Bindings[0].DescriptorCount = 1; // #todo_vk separate count in config for samplers
        TextureLayout.Bindings[0].DescriptorType = EDescriptorType::Sampler;
        
        auto& Sampler = TextureLayout.Bindings[0].ImmutableSamplers.emplace_back();
        Sampler.Backend = m_Desc.BackendVk;
        Sampler.DeviceHandle = m_Desc.PoolDesc.DeviceHandle;

        TextureLayout.Bindings[1].BindingIndex = 1;
        TextureLayout.Bindings[1].DescriptorCount = m_Desc.PoolDesc.DescriptorsCount - 1;
        TextureLayout.Bindings[1].DescriptorType = EDescriptorType::Texture;
        TextureLayout.Bindings[1].Flags |= EDescriptorSetLayoutFlags::Bindless;
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CBindlessDescriptorManagerVulkan::AddSystemVertexBindingsTo(CVertexLayoutDescription& VertexLayout)
{
    VertexLayout.Bindings.emplace_back(CVertexBindingDescription{
        .Index = (uint32)VertexLayout.Bindings.size(), // #todo_vk should this would remain constant? but if too high, register space would be wasted
        .Stride = sizeof(uint32),
        .BindingRate = EVertexBindingRate::Instance
        });
    VertexLayout.Attributes.emplace_back(CVertexAttributeDescription{
        .BindingIndex = VertexLayout.Bindings.back().Index,
        .SlotIndex = (uint32)VertexLayout.Attributes.size(),  // #todo_vk should this would remain constant? but if too high, register space would be wasted
        .Format = EFormatType::R32_UINT,
        .Offset = 0,
        });
    return C_STATUS::C_STATUS_OK;
}

CHandle<CResource> CResourceManagerVulkan::CreateResource()
{
    CHandle<CResource> Handle = m_Resources.Create();
    m_Desc.DeviceHandle.Fill(Handle);

    return Handle;
}

CHandle<CResource> CResourceManagerVulkan::CreateResource(const CResourceDesc& Desc)
{
    CHandle<CResource> Handle = CreateResource();

    C_STATUS Result = GetResource(Handle)->Init(Desc);
    CASSERT(C_SUCCEEDED(Result));
    if (C_SUCCEEDED(Result))
    {
        return Handle;
    }
    else
    {
        DestroyResource(Handle);
        return CHandle<CResource>();
    }
}

void CResourceManagerVulkan::DestroyResource(CHandle<CResource> Handle)
{
    m_Resources.Destroy(MoveTemp(Handle));
}

CResource* CResourceManagerVulkan::GetResource(CHandle<CResource> Handle)
{
    // #todo_mt #todo_vk should lock mutex and unlock after it finishes use of the resource
    return m_Resources.Get(MoveTemp(Handle));
}

const CResourceDesc* CResourceManagerVulkan::GetResourceDesc(CHandle<CResource> Handle)
{
    return &GetResource(Handle)->GetDesc();
}

CHandle<CShader> CResourceManagerVulkan::CreateShader()
{
    CHandle<CShader> Handle = m_Shaders.Create();
    m_Desc.DeviceHandle.Fill(Handle);

    return Handle;
}

CHandle<CShader> CResourceManagerVulkan::CreateShader(const CShaderDesc& Desc)
{
    CHandle<CShader> Handle = CreateShader();

    C_STATUS Result = GetShader(Handle)->Init(Desc);
    CASSERT(C_SUCCEEDED(Result));
    if (C_SUCCEEDED(Result))
    {
        return Handle;
    }
    else
    {
        DestroyShader(Handle);
        return CHandle<CShader>();
    }
}

void CResourceManagerVulkan::DestroyShader(CHandle<CShader> Handle)
{
    m_Shaders.Destroy(Handle);
}

CShader* CResourceManagerVulkan::GetShader(CHandle<CShader> Handle)
{
    return m_Shaders.Get(Handle);
}

const CShaderDesc* CResourceManagerVulkan::GetShaderDesc(CHandle<CShader> Handle)
{
    return &GetShader(Handle)->GetDesc();
}

CHandle<CResourceView> CResourceManagerVulkan::CreateResourceView()
{
    CHandle<CResourceView> Handle = m_ResourceViews.Create();
    m_Desc.DeviceHandle.Fill(Handle);

    return Handle;
}

CHandle<CResourceView> CResourceManagerVulkan::CreateResourceView(const CResourceViewDesc& Desc)
{
    CHandle<CResourceView> Handle = CreateResourceView();

    C_STATUS Result = GetResourceView(Handle)->Init(Desc);
    CASSERT(C_SUCCEEDED(Result));
    if (C_SUCCEEDED(Result))
    {
        return Handle;
    }
    else
    {
        DestroyResourceView(Handle);
        return CHandle<CResourceView>();
    }

    return Handle;
}

void CResourceManagerVulkan::DestroyResourceView(CHandle<CResourceView> Handle)
{
    m_ResourceViews.Destroy(Handle);
}

CResourceView* CResourceManagerVulkan::GetResourceView(CHandle<CResourceView> Handle)
{
    return m_ResourceViews.Get(Handle);
}

const CResourceViewDesc* CResourceManagerVulkan::GetResourceViewDesc(CHandle<CResourceView> Handle)
{
    return &GetResourceView(Handle)->GetDesc();
}

CHandle<CFence> CResourceManagerVulkan::CreateFence()
{
    CHandle<CFence> Handle = m_Fences.Create();
    m_Desc.DeviceHandle.Fill(Handle);

    return Handle;
}

CHandle<CFence> CResourceManagerVulkan::CreateFence(const CFenceDesc& Desc)
{
    CHandle<CFence> Handle = CreateFence();

    C_STATUS Result = GetFence(Handle)->Init(Desc);
    CASSERT(C_SUCCEEDED(Result));
    if (C_SUCCEEDED(Result))
    {
        return Handle;
    }
    else
    {
        DestroyFence(Handle);
        return CHandle<CFence>();
    }
}

void CResourceManagerVulkan::DestroyFence(CHandle<CFence> Handle)
{
    m_Fences.Destroy(Handle);
}

CFence* CResourceManagerVulkan::GetFence(CHandle<CFence> Handle)
{
    return m_Fences.Get(Handle);
}

const CFenceDesc* CResourceManagerVulkan::GetFenceDesc(CHandle<CFence> Handle)
{
    return &GetFence(Handle)->GetDesc();
}

C_STATUS CResourceManagerVulkan::AddSystemShaderBindingsTo(CShaderBindingSet& Bindings)
{
    CBindlessDescriptorManagerVulkan* BindlessManagerVk = GetBackendVk()->GetBindlessManagerVk(m_Desc.DeviceHandle);
    CASSERT(BindlessManagerVk);

    C_STATUS Result = BindlessManagerVk->AddSystemShaderBindingsTo(Bindings);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CResourceManagerVulkan::AddSystemVertexBindingsTo(CVertexLayoutDescription& VertexLayout)
{
    CBindlessDescriptorManagerVulkan* BindlessManagerVk = GetBackendVk()->GetBindlessManagerVk(m_Desc.DeviceHandle);
    CASSERT(BindlessManagerVk);

    C_STATUS Result = BindlessManagerVk->AddSystemVertexBindingsTo(VertexLayout);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;

}

void CResourceManagerVulkan::ReleaseDescriptorSetLayout(const CDescriptorSetLayout& Desc)
{
    auto it = m_DescriptorSetLayoutCache.find(Desc);
    if (it != m_DescriptorSetLayoutCache.end())
    {
        CASSERT(it->second.first > 0);
        if (--it->second.first == 0)
        {
            m_DescriptorSetLayoutCache.erase(it);
        }
    }
}

CHandle<CSampler> CResourceManagerVulkan::CreateSampler()
{
    CHandle<CSampler> Handle = m_Samplers.Create();
    m_Desc.DeviceHandle.Fill(Handle);

    return Handle;
}

CHandle<CSampler> CResourceManagerVulkan::CreateSampler(const CSamplerDesc& Desc)
{
    CHandle<CSampler> Handle = CreateSampler();

    C_STATUS Result = GetSampler(Handle)->Init(Desc);
    CASSERT(C_SUCCEEDED(Result));
    if (C_SUCCEEDED(Result))
    {
        return Handle;
    }
    else
    {
        DestroySampler(Handle);
        return CHandle<CSampler>();
    }
}

void CResourceManagerVulkan::DestroySampler(CHandle<CSampler> Handle)
{
    m_Samplers.Destroy(MoveTemp(Handle));
}

CSampler* CResourceManagerVulkan::GetSampler(CHandle<CSampler> Handle)
{
    return m_Samplers.Get(MoveTemp(Handle));
}

const CSamplerDesc* CResourceManagerVulkan::GetSamplerDesc(CHandle<CSampler> Handle)
{
    CSampler* Sampler = GetSampler(MoveTemp(Handle));
    return Sampler ? &Sampler->GetDesc() : nullptr;
}

CHandle<CPipelineState> CResourceManagerVulkan::CreatePipelineState()
{
    CHandle<CPipelineState> Handle = m_PipelineStates.Create();
    m_Desc.DeviceHandle.Fill(Handle);

    return Handle;
}

CHandle<CPipelineState> CResourceManagerVulkan::CreatePipelineState(const CPipelineStateDesc& Desc)
{
    CHandle<CPipelineState> Handle = CreatePipelineState();

    C_STATUS Result = GetPipelineState(Handle)->Init(Desc);
    CASSERT(C_SUCCEEDED(Result));
    if (C_SUCCEEDED(Result))
    {
        return Handle;
    }
    else
    {
        DestroyPipelineState(Handle);
        return CHandle<CPipelineState>();
    }
}

void CResourceManagerVulkan::DestroyPipelineState(CHandle<CPipelineState> Handle)
{
    m_PipelineStates.Destroy(MoveTemp(Handle));
}

CPipelineState* CResourceManagerVulkan::GetPipelineState(CHandle<CPipelineState> Handle)
{
    return m_PipelineStates.Get(MoveTemp(Handle));
}


const CPipelineStateDesc* CResourceManagerVulkan::GetPipelineStateDesc(CHandle<CPipelineState> Handle)
{
    CPipelineState* PipelineState = GetPipelineState(MoveTemp(Handle));
    return PipelineState ? &PipelineState->GetDesc() : nullptr;
}

CHandle<CPipelineState> CResourceManagerVulkan::GetPipelineStateCached(const CPipelineStateDesc& Desc)
{
    auto It = m_PipelineStatesCache.find(Desc);
    if (It != m_PipelineStatesCache.end())
    {
        ++It->second.first;
        return It->second.second;
    }

    auto& Data = m_PipelineStatesCache[Desc];
    Data.first = 1;

    auto& PipelineStateHandle = Data.second;
    PipelineStateHandle = CreatePipelineState(Desc);
    CASSERT(PipelineStateHandle.IsValid());

    return PipelineStateHandle;
}

void CResourceManagerVulkan::ReleasePipelineStateCached(CHandle<CPipelineState> Handle)
{
    CASSERT(Handle.IsValid());

    CPipelineState* PipelineState = GetPipelineState(Handle);
    C_ASSERT_RETURN(PipelineState);

    auto it = m_PipelineStatesCache.find(PipelineState->GetDesc());
    if (it != m_PipelineStatesCache.end())
    {
        CASSERT(it->second.first > 0);
        if (--it->second.first == 0)
        {
            m_PipelineStatesCache.erase(it);
            DestroyPipelineState(Handle);
        }
    }
}

CHandle<CSampler> CResourceManagerVulkan::GetSamplerCached(const CSamplerDesc& Desc)
{
    auto It = m_SamplersCache.find(Desc);
    if (It != m_SamplersCache.end())
    {
        ++It->second.first;
        return It->second.second;
    }

    auto& Data = m_SamplersCache[Desc];
    Data.first = 1;

    auto& PipelineStateHandle = Data.second;
    PipelineStateHandle = CreateSampler(Desc);
    CASSERT(PipelineStateHandle.IsValid());

    return PipelineStateHandle;
}

void CResourceManagerVulkan::ReleaseSamplerCached(CHandle<CSampler> Handle)
{
    CASSERT(Handle.IsValid());

    CSampler* PipelineState = GetSampler(Handle);
    C_ASSERT_RETURN(PipelineState);

    ReleaseSamplerCached(PipelineState->GetDesc());
}

void CResourceManagerVulkan::ReleaseSamplerCached(const CSamplerDesc& Desc)
{
    auto it = m_SamplersCache.find(Desc);
    if (it != m_SamplersCache.end())
    {
        CASSERT(it->second.first > 0);
        if (--it->second.first == 0)
        {
            DestroySampler(it->second.second);
            m_SamplersCache.erase(it);
        }
    }
}

} // namespace Cyclone::Render
