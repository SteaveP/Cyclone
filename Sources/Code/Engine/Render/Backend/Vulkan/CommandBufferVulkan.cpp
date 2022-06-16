#include "CommandBufferVulkan.h"

#include "CommandQueueVulkan.h"
#include "RenderBackendVulkan.h"
#include "BufferVulkan.h"

#include "Internal/RenderPassVk.h"
#include "Internal/FrameBufferVk.h"
#include "Internal/PipelineStateVk.h"

#include "Engine/Render/Types/Texture.h"

#include <type_traits>

namespace Cyclone::Render
{

CommandBufferVulkan::~CommandBufferVulkan()
{
    DeInit();
}

CCommandContextVulkan::~CCommandContextVulkan()
{
    DeInit();
}

C_STATUS CommandBufferVulkan::Init(CommandQueueVulkan* CommandQueue)
{
    m_CommandQueue = CommandQueue;

    VkDevice Device = m_CommandQueue->GetBackendVk()->GetGlobalContext().GetLogicalDevice(m_CommandQueue->GetDevice()).LogicalDeviceHandle;

    // Create semaphore
    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkResult Result = vkCreateSemaphore(Device, &SemaphoreInfo, nullptr, &m_CompleteSemaphore);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    // Create command pool
    VkCommandPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.queueFamilyIndex = m_CommandQueue->GetQueueFamilyIndex();
    PoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // Optional

    Result = vkCreateCommandPool(Device, &PoolInfo, nullptr, &m_CommandPool);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    // Create command buffer
    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = m_CommandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = 1;

    Result = vkAllocateCommandBuffers(Device, &AllocInfo, &m_CommandBuffer);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    // Command Context
    m_Context = MakeUnique<CCommandContextVulkan>();
    m_Context->Init(m_CommandQueue->GetBackendVk()->GetRenderer(), this);

    return C_STATUS::C_STATUS_OK;
}

void CommandBufferVulkan::DeInit()
{
    VkDevice Device = m_CommandQueue->GetBackendVk()->GetGlobalContext().GetLogicalDevice(m_CommandQueue->GetDevice()).LogicalDeviceHandle;

    if (m_CompleteSemaphore)
    {
        vkDestroySemaphore(Device, m_CompleteSemaphore, nullptr);
        m_CompleteSemaphore = VK_NULL_HANDLE;
    }

    if (m_CommandPool && m_CommandBuffer)
    {
        vkFreeCommandBuffers(Device, m_CommandPool, 1, &m_CommandBuffer);
        m_CommandBuffer = VK_NULL_HANDLE;
    }
    if (m_CommandPool)
    {
        vkDestroyCommandPool(Device, m_CommandPool, nullptr);
        m_CommandPool = VK_NULL_HANDLE;
    }
    
    CCommandBuffer::DeInit();
}

C_STATUS CommandBufferVulkan::Begin()
{
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult Result = vkBeginCommandBuffer(m_CommandBuffer, &BeginInfo);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
    return C_STATUS::C_STATUS_OK;
}

void CommandBufferVulkan::End()
{
    VkResult Result = vkEndCommandBuffer(m_CommandBuffer);
    C_ASSERT_VK_SUCCEEDED(Result);
}

void CommandBufferVulkan::Reset()
{
    VkDevice Device = m_CommandQueue->GetBackendVk()->GetGlobalContext().GetLogicalDevice(m_CommandQueue->GetDevice()).LogicalDeviceHandle;

    VkResult Result = vkResetCommandPool(Device, m_CommandPool, 0);
    C_ASSERT_VK_SUCCEEDED(Result);
}

void CommandBufferVulkan::Draw(uint32 IndexCount, uint32 InstanceCount, uint32 FirstIndex, int32 VertexOffset, uint32 FirstInstance)
{
    CCommandBuffer::Draw(IndexCount, InstanceCount, FirstIndex, VertexOffset, FirstInstance);
    vkCmdDrawIndexed(m_CommandBuffer, IndexCount, InstanceCount, FirstIndex, VertexOffset, FirstInstance);
}

void CCommandContextVulkan::BeginRenderPass(CCommandBuffer* CommandBuffer, const CRenderPass& RenderPass)
{
    CASSERT(m_ActiveRenderPassVk == nullptr);
    m_ActivePipelineState = nullptr;

    CCommandContext::BeginRenderPass(CommandBuffer, RenderPass);

    CommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CommandBufferVulkan);

    auto& Device = m_BackendVk->GetGlobalContext().GetLogicalDevice(CommandBufferVk->GetCommandQueue()->GetDevice());
    m_ActiveRenderPassVk = Device.ResourceManager->GetRenderPassVk(RenderPass);
    m_ActiveFrameBufferVk = Device.ResourceManager->GetFrameBufferVk(RenderPass);

    m_ActiveRenderPassVk->Begin(CommandBufferVk, m_ActiveFrameBufferVk, RenderPass);
}

void CCommandContextVulkan::EndRenderPass(CCommandBuffer* CommandBuffer)
{
    CASSERT(m_ActiveRenderPassVk);

    CommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CommandBufferVulkan);
    m_ActiveRenderPassVk->End(CommandBufferVk);

    m_ActiveRenderPassVk = nullptr;
    m_ActiveFrameBufferVk = nullptr;
    m_ActivePipelineState = nullptr;

    CCommandContext::EndRenderPass(CommandBuffer);
}

void CCommandContextVulkan::SetIndexBuffer(CCommandBuffer* CommandBuffer, CBuffer* IndexBuffer, uint32 Offset)
{
    CommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CommandBufferVulkan);
    CBufferVulkan* IndexBufferVk = BACKEND_DOWNCAST(IndexBuffer, CBufferVulkan);

    CASSERT(IndexBufferVk->GetDesc().Format == EFormatType::R16_UINT || IndexBufferVk->GetDesc().Format == EFormatType::R32_UINT);
    VkIndexType IndexType = IndexBufferVk->GetDesc().Format == EFormatType::R16_UINT ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
    vkCmdBindIndexBuffer(CommandBufferVk->Get(), IndexBufferVk->GetBuffer(), Offset, IndexType);
}

void CCommandContextVulkan::SetVertexBuffer(CCommandBuffer* CommandBuffer, CBuffer* VertexBuffer, uint32 Slot, uint64 Offset)
{
    CommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CommandBufferVulkan);
    CBufferVulkan* VertexBufferVk = BACKEND_DOWNCAST(VertexBuffer, CBufferVulkan);

    VkBuffer vertexBuffers[] = { VertexBufferVk->GetBuffer() };
    VkDeviceSize offests[] = { Offset };
    vkCmdBindVertexBuffers(CommandBufferVk->Get(), Slot, 1, vertexBuffers, offests);
}

Ptr<CPipeline> CCommandContextVulkan::SetPipeline(CCommandBuffer* CommandBuffer, const CPipelineDesc& Desc)
{
    CASSERT(Desc.Type != PipelineType::Graphics || m_ActiveRenderPassVk);
    CommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CommandBufferVulkan);

    Ptr<CPipeline> Pipepline;

    // find in cache or create
    auto& Device = m_BackendVk->GetGlobalContext().GetLogicalDevice(CommandBufferVk->GetCommandQueue()->GetDevice());
    m_ActivePipelineState = Device.ResourceManager->GetPipelineStateVk(m_ActiveRenderPassVk, Desc);

    C_STATUS Result = m_ActivePipelineState->Bind(CommandBufferVk);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), nullptr);

    return Pipepline;
}

C_STATUS CCommandContextVulkan::Init(IRenderer* Renderer, CCommandBuffer* CommandBuffer)
{
    C_STATUS Result = CCommandContext::Init(Renderer, CommandBuffer);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    m_BackendVk = static_cast<RenderBackendVulkan*>(m_Renderer->GetRendererBackend());
    m_CommandBufferVk = static_cast<CommandBufferVulkan*>(m_CommandBufferVk);

    return C_STATUS::C_STATUS_OK;
}

void CCommandContextVulkan::DeInit()
{
    CASSERT(m_ActiveRenderPassVk == nullptr && m_ActiveFrameBufferVk == nullptr);

    m_ActiveRenderPassVk = nullptr;

    CCommandContext::DeInit();
}

} //namespace Cyclone::Render
