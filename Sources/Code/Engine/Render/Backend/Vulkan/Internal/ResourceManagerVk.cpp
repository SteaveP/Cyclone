#include "ResourceManagerVk.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"

#include "Engine/Render/Types/Texture.h"

#include "RenderBackendVulkan.h"

#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"

#include "RenderPassVk.h"

namespace Cyclone::Render
{

#define HASH(Type, V) std::hash<Type>()(V)

CResourceManagerVk::CResourceManagerVk() = default;
CResourceManagerVk::~CResourceManagerVk()
{
    DeInit();

    CASSERT(m_DescriptorPool == VK_NULL_HANDLE);
}

C_STATUS CResourceManagerVk::Init(RenderBackendVulkan* RenderBackend, const ResourceManagerVkDesc& Desc)
{
    m_Desc = Desc;
    m_BackendVk = RenderBackend;

    C_STATUS Result = CreateDescriptorPool();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

void CResourceManagerVk::DeInit()
{
    DestroyDescriptorPool();

    m_FrameBufferCache.clear();
    m_RenderPassCache.clear();
    m_PipelineCache.clear();
}

C_STATUS CResourceManagerVk::CreateDescriptorPool()
{
    CASSERT(m_DescriptorPool == VK_NULL_HANDLE);

    VkDescriptorPoolSize PSize{};
    PSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    PSize.descriptorCount = 1000;

    VkDescriptorPoolCreateInfo DescPoolInfo{};
    DescPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    DescPoolInfo.maxSets = 1000;
    //DescPoolInfo.flags = VkDescriptorPoolCreateFlagBits:: ;
    DescPoolInfo.poolSizeCount = 1;
    DescPoolInfo.pPoolSizes = &PSize;
    VkResult ResultVk = vkCreateDescriptorPool(m_BackendVk->GetGlobalContext().GetLogicalDevice(m_Desc.DeviceHandle).LogicalDeviceHandle, &DescPoolInfo, nullptr, &m_DescriptorPool);
    C_ASSERT_VK_SUCCEEDED(ResultVk);

    return C_STATUS::C_STATUS_OK;
}

void CResourceManagerVk::DestroyDescriptorPool()
{
    if (m_DescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(m_BackendVk->GetGlobalContext().GetLogicalDevice(m_Desc.DeviceHandle).LogicalDeviceHandle, m_DescriptorPool, nullptr);
        m_DescriptorPool = VK_NULL_HANDLE;
    }
}


RenderPassVk* CResourceManagerVk::GetRenderPassVk(const CRenderPass& RenderPass)
{
    return GetOrCreateRenderPassVk(RenderPass);
}

FrameBufferVk* CResourceManagerVk::GetFrameBufferVk(const CRenderPass& RenderPass)
{
    return GetOrCreateFrameBufferVk(RenderPass);
}
PipelineStateVk* CResourceManagerVk::GetPipelineStateVk(RenderPassVk* RenderPass, const CPipelineDesc& PipelineDesc)
{
    return GetOrCreatePipelineStateVk(RenderPass, PipelineDesc);
}

RenderPassVk* CResourceManagerVk::GetOrCreateRenderPassVk(const CRenderPass& RenderPass)
{
    CRenderPass RenderPassToFind = RenderPass;
    {
        // Remove all refs to textures
        RenderPassToFind.RenderTargetSet.DepthScentil.RenderTarget = nullptr;

        for (uint32 i = 0; i < CRenderTargetSet::MaxRenderTargets; ++i)
        {
            RenderPassToFind.RenderTargetSet.RenderTargets[i].RenderTarget = nullptr;
        }
    }

    auto It = m_RenderPassCache.find(RenderPassToFind);
    if (It != m_RenderPassCache.end())
        return It->second.get();

    auto& RenderPassVkPtr = m_RenderPassCache[RenderPassToFind];
    RenderPassVkPtr = MakeUnique<RenderPassVk>();

    C_STATUS Result = RenderPassVkPtr->Init(m_BackendVk, m_Desc.DeviceHandle, RenderPass);
    CASSERT(C_SUCCEEDED(Result));

    return RenderPassVkPtr.get();
}

FrameBufferVk* CResourceManagerVk::GetOrCreateFrameBufferVk(const CRenderPass& RenderPass)
{
    RenderPassVk* RenderPassVkPtr = GetOrCreateRenderPassVk(RenderPass);
    C_ASSERT_RETURN_VAL(RenderPassVkPtr, nullptr);
    
    CFrameBufferHandle FrameBufferHandle = CFrameBufferHandle::Create(RenderPass);

    auto It = m_FrameBufferCache.find(FrameBufferHandle);
    if (It != m_FrameBufferCache.end())
        return It->second.get();

    auto& FrameBuffer = m_FrameBufferCache[FrameBufferHandle];
    FrameBuffer = MakeUnique<FrameBufferVk>();

    C_STATUS Result = FrameBuffer->Init(m_Desc.DeviceHandle, m_BackendVk, RenderPassVkPtr, RenderPass);
    CASSERT(C_SUCCEEDED(Result));

    return FrameBuffer.get();
}

PipelineStateVk* CResourceManagerVk::GetOrCreatePipelineStateVk(RenderPassVk* RenderPass, const CPipelineDesc& PipelineDesc)
{
    CPipelineHandle PipelineDescHandle = CPipelineHandle::Create(PipelineDesc);

    auto It = m_PipelineCache.find(PipelineDescHandle);
    if (It != m_PipelineCache.end())
        return It->second.get();

    auto& PipelineStateVkPtr = m_PipelineCache[PipelineDescHandle];
    PipelineStateVkPtr = MakeUnique<PipelineStateVk>();

    PipelineStateVkInitInfo Info{};
    Info.Backend = m_BackendVk;
    Info.DeviceHandle = m_Desc.DeviceHandle;
    Info.RenderPass = RenderPass;
    Info.PipelineDesc = &PipelineDesc;

    C_STATUS Result = PipelineStateVkPtr->Init(Info);
    CASSERT(C_SUCCEEDED(Result));

    return PipelineStateVkPtr.get();
}

std::size_t RenderPassHasher::operator()(const CRenderPass& Key) const noexcept
{
    std::size_t Value = HASH(uint32, Key.RenderTargetSet.RenderTargetsCount) << 32;

    auto MakeHash = [&](const auto& Slot, uint32 i)
    {
        std::size_t Result = 0;
        Result ^= (HASH(void*, Slot.RenderTarget.get()) >> (5 + i));
        Result ^= (HASH(uint32, (uint32)Slot.InitialLayout) >> (20 + i));
        Result ^= (HASH(uint32, (uint32)Slot.Layout) >> (15 + i));
        Result ^= (HASH(uint32, (uint32)Slot.FinalLayout) >> (10 + i));
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

CPipelineHandle CPipelineHandle::Create(const CPipelineDesc& Desc)
{
    CPipelineHandle Handle{};

    Handle.Type = Desc.Type;
    Handle.Flags = Desc.Flags;
    Handle.PrimitiveTopology = Desc.PrimitiveTopology;
    Handle.Rasterizer = Desc.Rasterizer;
    Handle.DepthStencil = Desc.DepthStencil;
    Handle.Blend = Desc.Blend;

    Handle.VertexShader = Desc.VertexShader.get();
    Handle.PixelShader = Desc.PixelShader.get();
    Handle.ComputeShader = Desc.ComputeShader.get();

    Handle.DeviceHandle = Desc.DeviceHandle;
    Handle.RendererBackend = Desc.RendererBackend;

    return Handle;
}

std::size_t PipelineHasher::operator()(const CPipelineHandle& Key) const noexcept
{
    std::size_t Value = 0;

    Value ^= (HASH(uint32, (uint32)Key.Type));
    Value ^= (HASH(uint32, (uint32)Key.Flags));
    Value ^= (HASH(uint32, (uint32)Key.PrimitiveTopology));
    Value ^= (HASH(uint32, (uint32)Key.Rasterizer) >> 1);
    Value ^= (HASH(uint32, (uint32)Key.DepthStencil) << 6);
    Value ^= (HASH(uint32, (uint32)Key.Blend));

    Value ^= (HASH(uint64, (uint64)Key.VertexShader) << 3);
    Value ^= (HASH(uint64, (uint64)Key.PixelShader));
    Value ^= (HASH(uint64, (uint64)Key.ComputeShader) >> 1);

    Value ^= (HASH(uint32, (uint32)Key.DeviceHandle.LogicalDeviceHandle) << (16));
    Value ^= (HASH(uint32, (uint32)Key.DeviceHandle.PhysicalDeviceHandle) << (24));
    Value ^= (HASH(uint64, (uint64)Key.RendererBackend));

    return Value;
}

CFrameBufferHandle CFrameBufferHandle::Create(const CRenderPass& RenderPass)
{
    CFrameBufferHandle Handle{};

    // Fill ptr to needed resources
    for (uint32 i = 0; i < RenderPass.RenderTargetSet.RenderTargetsCount; ++i)
    {
        Handle.AttachmentHandles[i] = RenderPass.RenderTargetSet.RenderTargets[i].RenderTarget->RenderTargetView->PlatformDataPtr;
    }

    if (RenderPass.RenderTargetSet.DepthScentil.RenderTarget)
    {
        Handle.AttachmentHandles[RenderPass.RenderTargetSet.RenderTargetsCount] = RenderPass.RenderTargetSet.DepthScentil.RenderTarget->RenderTargetView->PlatformDataPtr;
    }

    CTexture* TextureRef = nullptr;
    if (RenderPass.RenderTargetSet.RenderTargetsCount > 0)
        TextureRef = RenderPass.RenderTargetSet.RenderTargets[0].RenderTarget->Texture.get();
    if (TextureRef == nullptr && RenderPass.RenderTargetSet.DepthScentil.RenderTarget)
        TextureRef = RenderPass.RenderTargetSet.DepthScentil.RenderTarget->Texture.get();
    CASSERT(TextureRef);

    Handle.Width = static_cast<uint32>(TextureRef->GetDesc().Width);
    Handle.Height = TextureRef->GetDesc().Height;
    Handle.Layers = 1;

    return Handle;
}

std::size_t CFrameBufferHandleHasher::operator()(const CFrameBufferHandle& Key) const noexcept
{
    std::size_t Value = (HASH(uint32, Key.Width) << 10) ^ (HASH(uint32, Key.Height) << 20) ^ (HASH(uint32, Key.Layers) << 30);

    for (uint32 i = 0; i < Key.AttachmentHandlesCount; ++i)
    {
        Value ^= (HASH(void*, Key.AttachmentHandles[i]) >> i);
    }

    return Value;
}

} // namespace Cyclone::Render
