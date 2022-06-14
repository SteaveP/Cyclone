#include "CommandBufferVulkan.h"

#include "CommandQueueVulkan.h"
#include "RenderBackendVulkan.h"

#include "Internal/RenderPassVk.h"
#include "Internal/FrameBufferVk.h"

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
    m_Context = std::make_unique<CCommandContextVulkan>();
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

void CCommandContextVulkan::BeginRenderPass(CCommandBuffer* CommandBuffer, const CRenderPass& RenderPass)
{
    CASSERT(m_ActiveRenderPassVk == nullptr);

    CCommandContext::BeginRenderPass(CommandBuffer, RenderPass);

    CommandBufferVulkan* CommandBufferVk = reinterpret_cast<CommandBufferVulkan*>(CommandBuffer);
    WindowContextVulkan* WindowContextVk = reinterpret_cast<WindowContextVulkan*>(m_Renderer->GetDefaultWindowContext());

    m_ActiveRenderPassVk = GetOrCreateRenderPass(CommandBufferVk, RenderPass);
    m_ActiveFrameBufferVk = GetOrCreateFrameBuffer(CommandBufferVk, WindowContextVk, RenderPass);

    RenderPassVkBeginInfo BeginInfo{};
    BeginInfo.FrameBuffer = m_ActiveFrameBufferVk;
    BeginInfo.RenderArea.offset = { (int32)RenderPass.ViewportExtent.X,  (int32)RenderPass.ViewportExtent.Y };
    BeginInfo.RenderArea.extent = { (uint32)(RenderPass.ViewportExtent.Z - RenderPass.ViewportExtent.X), (uint32)(RenderPass.ViewportExtent.W - RenderPass.ViewportExtent.Y) };

    BeginInfo.ClearColorsCount = 0;
    for (uint32 i = 0; i < RenderPass.RenderTargetSet.RenderTargetsCount; ++i)
    {
        const auto& ClearValue = RenderPass.RenderTargetSet.RenderTargets[i].ClearValue;
        if (!ClearValue.has_value())
            break;

        BeginInfo.ClearColors[BeginInfo.ClearColorsCount].color = VkClearColorValue{ 
            ClearValue->Color.X, ClearValue->Color.Y, ClearValue->Color.Z, ClearValue->Color.W };

        ++BeginInfo.ClearColorsCount;
    }

    if (RenderPass.RenderTargetSet.DepthScentil.ClearValue.has_value())
    {
        const auto& ClearValue = RenderPass.RenderTargetSet.DepthScentil.ClearValue;

        BeginInfo.ClearColors[BeginInfo.ClearColorsCount].color = VkClearColorValue{
            ClearValue->Color.X, ClearValue->Color.Y, ClearValue->Color.Z, ClearValue->Color.W };

        ++BeginInfo.ClearColorsCount;
    }

    m_ActiveRenderPassVk->Begin(CommandBufferVk, BeginInfo);
}

void CCommandContextVulkan::EndRenderPass(CCommandBuffer* CommandBuffer)
{
    CASSERT(m_ActiveRenderPassVk);

    CommandBufferVulkan* CommandBufferVk = reinterpret_cast<CommandBufferVulkan*>(CommandBuffer);
    m_ActiveRenderPassVk->End(CommandBufferVk);

    m_ActiveRenderPassVk = nullptr;
    m_ActiveFrameBufferVk = nullptr;

    CCommandContext::EndRenderPass(CommandBuffer);
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

    m_FrameBufferCache.clear();
    m_RenderPassCache.clear();

    CCommandContext::DeInit();
}

RenderPassVk* CCommandContextVulkan::GetOrCreateRenderPass(CommandBufferVulkan* CommandBufferVk, const CRenderPass& RenderPass)
{
    // #todo_vk refactor
    uint64 HashRP = 0;

    if (m_RenderPassCache.find(HashRP) == m_RenderPassCache.end())
    {
        RenderPassVkInitInfo Desc{};
        RenderPassVkInitInfo::CreateFromRenderPass(Desc, m_BackendVk, CommandBufferVk->GetCommandQueue()->GetDevice(), RenderPass);

        auto& RenderPass = m_RenderPassCache[HashRP];
        RenderPass = std::make_unique<RenderPassVk>();

        C_STATUS Result = RenderPass->Init(Desc);
        CASSERT(C_SUCCEEDED(Result));
    }
    return m_RenderPassCache[HashRP].get();
}

FrameBufferVk* CCommandContextVulkan::GetOrCreateFrameBuffer(CommandBufferVulkan* CommandBufferVk, const WindowContextVulkan* WindowContextVk, const CRenderPass& RenderPass)
{
    // #todo_vk refactor
    uint64 HashFB = WindowContextVk->GetCurrentImageIndex();
    if (m_FrameBufferCache.find(HashFB) == m_FrameBufferCache.end())
    {
        FrameBufferVkInitInfo FBDesc{};
        FBDesc.Backend = m_BackendVk;
        FBDesc.Device = CommandBufferVk->GetCommandQueue()->GetDevice();
        FBDesc.RenderPass = m_ActiveRenderPassVk;
        FBDesc.Width = WindowContextVk->GetSwapchainExtent().width;
        FBDesc.Height = WindowContextVk->GetSwapchainExtent().height;
        FBDesc.Layers = 1;

        FBDesc.AttachmentsCount = RenderPass.RenderTargetSet.RenderTargetsCount
            + (RenderPass.RenderTargetSet.DepthScentil.RenderTarget ? 1 : 0);

        for (uint32 i = 0; i < RenderPass.RenderTargetSet.RenderTargetsCount; ++i)
        {
            // #todo_vk refactor
            FBDesc.Attachments[i] = WindowContextVk->GetSwapchainImageView(WindowContextVk->GetCurrentImageIndex());
        }

        if (RenderPass.RenderTargetSet.DepthScentil.RenderTarget)
        {
            CASSERT(false);
        }

        auto& FrameBuffer = m_FrameBufferCache[HashFB];
        FrameBuffer = std::make_unique<FrameBufferVk>();

        C_STATUS Result = FrameBuffer->Init(FBDesc);
        CASSERT(C_SUCCEEDED(Result));
    }

    return m_FrameBufferCache[HashFB].get();
}

} //namespace Cyclone::Render
