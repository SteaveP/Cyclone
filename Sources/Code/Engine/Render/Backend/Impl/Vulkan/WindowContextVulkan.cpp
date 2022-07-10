#include "WindowContextVulkan.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"
#include "Engine/Framework/IRenderer.h"
#include "Engine/Utils/Log.h"

#include "Engine/Render/CommonRender.h"
#include "Engine/Render/Backend/Resource.h"
#include "Engine/Render/Backend/ResourceView.h"

#include "RenderBackendVulkan.h"
#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"

#if PLATFORM_WIN64
    #include <vulkan/vulkan_win32.h>
#else
    #error unsupported platform
#endif

namespace Cyclone::Render
{

CWindowContextVulkan::CWindowContextVulkan() = default;
CWindowContextVulkan::CWindowContextVulkan(CWindowContextVulkan&& Other) noexcept : CWindowContext(MoveTemp(Other))
{
    std::swap(m_Backend, Other.m_Backend);

    std::swap(m_Surface, Other.m_Surface);
    std::swap(m_Swapchain, Other.m_Swapchain);

    std::swap(m_MinSwapchainImageCount, Other.m_MinSwapchainImageCount);
    std::swap(m_SwapchainImages, Other.m_SwapchainImages);
    std::swap(m_SwapchainImageFormat, Other.m_SwapchainImageFormat);
    std::swap(m_SwapchainExtent, Other.m_SwapchainExtent);

    std::swap(m_FramebufferResized, Other.m_FramebufferResized);

    std::swap(m_ImageAvailabeSemaphores, Other.m_ImageAvailabeSemaphores);
    std::swap(m_RenderFinishedSemaphores, Other.m_RenderFinishedSemaphores);
    std::swap(m_InflightFences, Other.m_InflightFences);
    std::swap(m_ImagesInFlight, Other.m_ImagesInFlight);

    std::swap(m_CurrentMsaaSamples, Other.m_CurrentMsaaSamples);
}
CWindowContextVulkan& CWindowContextVulkan::operator=(CWindowContextVulkan&& Other) noexcept
{
    if (this != &Other)
    {
        CWindowContext::operator=(MoveTemp(Other));
        std::swap(m_Backend, Other.m_Backend);

        std::swap(m_Surface, Other.m_Surface);
        std::swap(m_Swapchain, Other.m_Swapchain);

        std::swap(m_MinSwapchainImageCount, Other.m_MinSwapchainImageCount);
        std::swap(m_SwapchainImages, Other.m_SwapchainImages);
        std::swap(m_SwapchainImageFormat, Other.m_SwapchainImageFormat);
        std::swap(m_SwapchainExtent, Other.m_SwapchainExtent);

        std::swap(m_FramebufferResized, Other.m_FramebufferResized);

        std::swap(m_ImageAvailabeSemaphores, Other.m_ImageAvailabeSemaphores);
        std::swap(m_RenderFinishedSemaphores, Other.m_RenderFinishedSemaphores);
        std::swap(m_InflightFences, Other.m_InflightFences);
        std::swap(m_ImagesInFlight, Other.m_ImagesInFlight);

        std::swap(m_CurrentMsaaSamples, Other.m_CurrentMsaaSamples);
    }
    return *this;
}
CWindowContextVulkan::~CWindowContextVulkan()
{
    DeInitImpl();

    CASSERT(m_Surface == VK_NULL_HANDLE);
}

C_STATUS CWindowContextVulkan::Init(IRenderer* Renderer, IWindow* Window)
{
    C_STATUS Result = CWindowContext::Init(Renderer, Window);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    m_Backend = GET_BACKEND_IMPL(Renderer->GetRendererBackend());
    
    if (m_Backend == nullptr || m_Window == nullptr)
        return C_STATUS::C_STATUS_INVALID_ARG;

    Result = CreateSurface(m_Window);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result); 
    
    Result = m_Backend->GetDeviceManager().GetOrCreateDevice(m_Surface, m_DeviceHandle);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    for (uint32 i = 0; i < (uint32)CommandQueueType::Count; ++i)
    {
        m_CommandQueueCache[i] = m_Backend->GetDeviceManager().GetDevice(m_DeviceHandle).GetCommandQueue((CommandQueueType)i);
    }

    Result = CreateSwapchain();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = CreateSwapchainImageViews();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = CreateSyncObjects();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    LOG_INFO("Render: Context for Window \"{}\" created on Device ({},{}) \"{}\"", Window->GetName(), 
        (uint32)m_DeviceHandle.PhysDeviceHandle, (uint32)m_DeviceHandle.DeviceHandle, m_Backend->GetDeviceManager().GetPhysDevice(m_DeviceHandle).Name);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CWindowContextVulkan::DeInit()
{
    DeInitImpl();

    C_STATUS Result = CWindowContext::DeInit();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

void CWindowContextVulkan::DeInitImpl() noexcept
{
    if (m_Backend)
    {
        DestroySyncObjects(true);
        CleanupSwapchain(true);
        DestroySurface(true);

        m_Backend->GetDeviceManager().ReleaseDevice(m_DeviceHandle);
        m_Backend = nullptr;
    }
}

C_STATUS CWindowContextVulkan::CreateSurface(IWindow* Window)
{
    // #todo_vk make this platform independent (move to IPlatformRenerer like ImGUI Module)
#if PLATFORM_WIN64
    VkWin32SurfaceCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    CreateInfo.hwnd = (HWND)Window->GetPlatformWindowHandle();
    CreateInfo.hinstance = GetModuleHandle(nullptr);

    VkResult Result = vkCreateWin32SurfaceKHR(m_Backend->GetDeviceManager().GetInstanceVk(), &CreateInfo, nullptr, &m_Surface);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
#else
    #error Unsupported platform
#endif

    return C_STATUS::C_STATUS_OK;
}

void CWindowContextVulkan::DestroySurface(bool IsDeferred)
{
    if (m_Surface)
    {
        auto Destroy = [Backend = m_Backend, Surface = m_Surface]()
        {
            vkDestroySurfaceKHR(Backend->GetDeviceManager().GetInstanceVk(), Surface, nullptr);
        };

        if (IsDeferred)
            m_Backend->GetDisposalManagerVk(m_DeviceHandle)->AddDisposable(Destroy);
        else
            Destroy();
    }

    m_Surface = nullptr;
}

void CWindowContextVulkan::CleanupSwapchain(bool IsDeferred, bool IsRecreating)
{
    const auto& Device = m_Backend->GetDeviceManager().GetDevice(m_DeviceHandle);
    
    for (size_t i = 0; i < m_BackBuffers.size(); ++i)
    {
        auto& BackBuffer = m_BackBuffers[i];
        if (BackBuffer.Texture.IsValid())
            Device.ResourceManager->DestroyResource(
                std::exchange(BackBuffer.Texture, decltype(BackBuffer.Texture){}));

        if (BackBuffer.RenderTargetView.IsValid())
            Device.ResourceManager->DestroyResourceView(BackBuffer.RenderTargetView);
        if (BackBuffer.DepthStencilView.IsValid())
            Device.ResourceManager->DestroyResourceView(BackBuffer.DepthStencilView);
        if (BackBuffer.ShaderResourceView.IsValid())
            Device.ResourceManager->DestroyResourceView(BackBuffer.ShaderResourceView);
        if (BackBuffer.UnorderedAccessView.IsValid())
            Device.ResourceManager->DestroyResourceView(BackBuffer.UnorderedAccessView);
    }
    m_BackBuffers.clear();

    if (m_Swapchain && IsRecreating == false)
    {
        auto Destroy = [Backend = m_Backend, DeviceHandle = m_DeviceHandle, Swapchain = m_Swapchain]()
        {
            const auto& Device = Backend->GetDeviceManager().GetDevice(DeviceHandle);
            VK_CALL(Device, vkDestroySwapchainKHR(Device.DeviceVk, Swapchain, nullptr));
        };

        if (IsDeferred)
            m_Backend->GetDisposalManagerVk(m_DeviceHandle)->AddDisposable(Destroy);
        else
            Destroy();

        m_Swapchain = VK_NULL_HANDLE;
    }
}

C_STATUS CWindowContextVulkan::RecreateSwapchain()
{
    int Width = m_Window->GetWidth();
    int Height = m_Window->GetHeight();

    const CPhysicalDevice& PhysDevice = m_Backend->GetDeviceManager().GetPhysDevice(m_DeviceHandle);

    // We must query actual swap chain support because it contains info about size that's needed in case of window resizing
    const CSwapChainSupportDetails& SwapChainSupport = m_Backend->GetDeviceManager().QuerySwapChainSupport(PhysDevice.DeviceVk, m_Surface);
    const auto& CurrentExtent = SwapChainSupport.Capabilities.currentExtent;

    if (Width > 0 && Height > 0 && CurrentExtent.width > 0 && CurrentExtent.height > 0)
    {
        const auto& Device = m_Backend->GetDeviceManager().GetDevice(m_DeviceHandle);
        VK_CALL(Device, vkDeviceWaitIdle(Device.DeviceVk));

        CleanupSwapchain(false, true);

        C_STATUS Result = CreateSwapchain();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
        Result = CreateSwapchainImageViews();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
        return C_STATUS::C_STATUS_OK;
    }

    return C_STATUS::C_STATUS_INVALID_ARG;
}

C_STATUS CWindowContextVulkan::CreateSwapchainImageViews()
{
    const auto& Device = m_Backend->GetDeviceManager().GetDevice(m_DeviceHandle);

    for (size_t i = 0; i < m_BackBuffers.size(); ++i)
    {
        auto& BackBuffer = m_BackBuffers[i];

        CResource* BackBufferTex = Device.ResourceManager->GetResource(BackBuffer.Texture);
        CASSERT(BackBufferTex);

        CResourceViewDesc Desc{};
        Desc.Backend = m_Backend;
        Desc.Type = EResourceFlags::Texture;
        Desc.Resource = BackBuffer.Texture;
        Desc.Format = BackBufferTex->GetDesc().Format;
        Desc.Texture.ViewType = ETextureViewType::Type2D;
        Desc.Texture.AspectMask = EImageAspectType::Color;

        BackBuffer.RenderTargetView = Device.ResourceManager->CreateResourceView(Desc);
    }

    return C_STATUS::C_STATUS_OK;
}

VkSurfaceFormatKHR CWindowContextVulkan::ChooseSwapSurfaceFormat(const Vector<VkSurfaceFormatKHR>& AvailableFormats)
{
    for (const auto& AvailableFormat : AvailableFormats)
    {
        if (AvailableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && AvailableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return AvailableFormat;
        }
    }

    return AvailableFormats[0];
}

VkPresentModeKHR CWindowContextVulkan::ChooseSwapPresentMode(const Vector<VkPresentModeKHR>& AvailablePresentModes)
{
    for (const auto& AvailablePresentMode : AvailablePresentModes) {
        if (AvailablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return AvailablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D CWindowContextVulkan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities)
{
    if (Capabilities.currentExtent.width != UINT32_MAX)
    {
        return Capabilities.currentExtent;
    }
    else
    {
        int Width = m_Window->GetWidth();
        int Height = m_Window->GetHeight();
        VkExtent2D ActualExtent = { static_cast<uint32_t>(Width), static_cast<uint32_t>(Height) };

        ActualExtent.width = C_MAX(Capabilities.minImageExtent.width, C_MIN(Capabilities.maxImageExtent.width, ActualExtent.width));
        ActualExtent.height = C_MAX(Capabilities.minImageExtent.height, C_MIN(Capabilities.maxImageExtent.height, ActualExtent.height));

        return ActualExtent;
    }
}

C_STATUS CWindowContextVulkan::CreateSwapchain()
{
    const CPhysicalDevice& PhysDevice = m_Backend->GetDeviceManager().GetPhysDevice(m_DeviceHandle);
    const CDevice& Device = m_Backend->GetDeviceManager().GetDevice(m_DeviceHandle);

    // We must query actual swap chain support because it contains info about size that's needed in case of window resizing
    const CSwapChainSupportDetails& SwapChainSupport = m_Backend->GetDeviceManager().QuerySwapChainSupport(PhysDevice.DeviceVk, m_Surface);

    VkSurfaceFormatKHR SurfaceFormat = ChooseSwapSurfaceFormat(SwapChainSupport.Formats);
    VkPresentModeKHR PresentMode = ChooseSwapPresentMode(SwapChainSupport.PresentModes);
    VkExtent2D Extent = ChooseSwapExtent(SwapChainSupport.Capabilities);

    m_MinSwapchainImageCount = SwapChainSupport.Capabilities.minImageCount;
    uint32_t ImagesCount = SwapChainSupport.Capabilities.minImageCount + 1;

    if (SwapChainSupport.Capabilities.maxImageCount > 0 && ImagesCount > SwapChainSupport.Capabilities.maxImageCount)
    {
        ImagesCount = SwapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    CreateInfo.surface = m_Surface;
    CreateInfo.minImageCount = ImagesCount;
    CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    CreateInfo.imageFormat = SurfaceFormat.format;
    CreateInfo.imageExtent = Extent;
    CreateInfo.imageArrayLayers = 1;
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const CQueueFamilyIndices& Indices = PhysDevice.Features.QueueFamilyIndices;
    uint32_t QueueFamilyIndices[] = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

    if (Indices.GraphicsFamily != Indices.PresentFamily)
    {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        CreateInfo.queueFamilyIndexCount = 2;
        CreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
    }
    else
    {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        CreateInfo.queueFamilyIndexCount = 0; // optional
        CreateInfo.pQueueFamilyIndices = nullptr; // optional
    }

    CreateInfo.preTransform = SwapChainSupport.Capabilities.currentTransform;
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    CreateInfo.presentMode = PresentMode;
    CreateInfo.clipped = VK_TRUE;
    CreateInfo.oldSwapchain = m_Swapchain;

    VkResult ResultVk = VK_CALL(Device, vkCreateSwapchainKHR(Device.DeviceVk, &CreateInfo, nullptr, &m_Swapchain));
    C_ASSERT_VK_SUCCEEDED(ResultVk);

    // acquire swap chain images
    uint32_t SwapchainImagesCount;
    VK_CALL(Device, vkGetSwapchainImagesKHR(Device.DeviceVk, m_Swapchain, &SwapchainImagesCount, nullptr));
    m_SwapchainImages.resize(SwapchainImagesCount);
    VK_CALL(Device, vkGetSwapchainImagesKHR(Device.DeviceVk, m_Swapchain, &SwapchainImagesCount, m_SwapchainImages.data()));

    m_SwapchainImageFormat = SurfaceFormat.format;
    m_SwapchainExtent = Extent;

    m_BackBuffers.resize(m_SwapchainImages.size());
    for (size_t i = 0; i < m_BackBuffers.size(); ++i)
    {
        auto& BackBuffer = m_BackBuffers[i];

        CResourceDesc Desc{};
        Desc.Format = ConvertFormatType(CreateInfo.imageFormat);
        Desc.Flags = EResourceFlags::Texture;
        Desc.InitialLayout = EImageLayoutType::Undefined;
        Desc.Texture.InitialUsage = EImageUsageType::None;
        Desc.Texture.ImageType = ETextureType::Type2D;
        Desc.Texture.Tiling = ETilingType::Optimal;
        Desc.Texture.Width = CreateInfo.imageExtent.width;
        Desc.Texture.Height = CreateInfo.imageExtent.height;
        Desc.Texture.Depth = 1;
        Desc.Texture.MipLevels = 1;
        Desc.Texture.ArrayCount = 1;
        Desc.Texture.SamplesCount = 1;
        Desc.Backend = m_Backend;
        Desc.DeviceHandle = m_DeviceHandle;
        Desc.ExternalBackendResource = m_SwapchainImages[i];
#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.Name = "SwapChain Image " + ToString(i);
#endif

        BackBuffer.Texture = Device.ResourceManager->CreateResource(Desc);
        C_ASSERT_RETURN_VAL(BackBuffer.Texture.IsValid(), C_STATUS::C_STATUS_ERROR);
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CWindowContextVulkan::CreateSyncObjects()
{
    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo FenceInfo{};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult Result;

    const auto& Device = m_Backend->GetDeviceManager().GetDevice(m_DeviceHandle);
    for (size_t i = 0; i < m_Backend->GetRenderer()->GetFramesInFlightCount(); ++i)
    {
        Result = VK_CALL(Device, vkCreateSemaphore(Device.DeviceVk, &SemaphoreInfo, nullptr, &m_ImageAvailabeSemaphores[i]));
        C_ASSERT_VK_SUCCEEDED(Result);

        Result = VK_CALL(Device, vkCreateSemaphore(Device.DeviceVk, &SemaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));
        C_ASSERT_VK_SUCCEEDED(Result);

        Result = VK_CALL(Device, vkCreateFence(Device.DeviceVk, &FenceInfo, nullptr, &m_InflightFences[i]));
        C_ASSERT_VK_SUCCEEDED(Result);

#if ENABLE_DEBUG_RENDER_BACKEND
        SetDebugNameVk("ImageAvailableSemaphore" + ToString(i), VK_OBJECT_TYPE_SEMAPHORE, (uint64)m_ImageAvailabeSemaphores[i], Device);
        SetDebugNameVk("RenderFinishedSemaphore" + ToString(i), VK_OBJECT_TYPE_SEMAPHORE, (uint64)m_RenderFinishedSemaphores[i], Device);
        SetDebugNameVk("InflightFence" + ToString(i), VK_OBJECT_TYPE_FENCE, (uint64)m_InflightFences[i], Device);
#endif
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CWindowContextVulkan::BeginRender()
{
    C_STATUS Result = CWindowContext::BeginRender();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    const auto& Device = m_Backend->GetDeviceManager().GetDevice(m_DeviceHandle);

    VK_CALL(Device, vkWaitForFences(Device.DeviceVk, 1, &m_InflightFences[m_CurrentLocalFrame], VK_TRUE, UINT64_MAX));

    // #todo_vk #todo_vk_async_compute other command queues as well?
    Device.GetCommandQueue(CommandQueueType::Graphics)->OnBeginRender();

    VkResult ResultVk = VK_CALL(Device, vkAcquireNextImageKHR(Device.DeviceVk, m_Swapchain, UINT64_MAX, m_ImageAvailabeSemaphores[m_CurrentLocalFrame], VK_NULL_HANDLE, &m_CurrentImageIndex));

    if (ResultVk == VK_ERROR_OUT_OF_DATE_KHR || ResultVk == VkResult::VK_SUBOPTIMAL_KHR)
    {
#if 0
        // We should explicitly wait semaphore in this case? according to
        // according to https://stackoverflow.com/questions/59825832/vulkan-why-do-we-need-to-check-for-window-resize-after-vkqueuepresentkhr
        // and https://github.com/KhronosGroup/Vulkan-Docs/issues/1059
        if (CCommandQueueVulkan* CommandQueueVk = GetCommandQueueVk(CommandQueueType::Graphics))
        {
            Result = CommandQueueVk->SubmitVk(nullptr, 0, m_ImageAvailabeSemaphores[m_CurrentLocalFrame], nullptr);
            CASSERT(C_SUCCEEDED(Result));
        }
#endif

        Result = RecreateSwapchain();
        // return only if resizing was successful
        if (Result == C_STATUS::C_STATUS_OK)
        {
            return C_STATUS::C_STATUS_OK;
        }
        else if (Result == C_STATUS::C_STATUS_INVALID_ARG)
        {
            // Manually signal semaphore to be sure that frame will be processed anyway
            if (CCommandQueueVulkan* CommandQueueVk = GetCommandQueueVk(CommandQueueType::Graphics))
            {
                VkSubmitInfo2 Info{ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
                Info.signalSemaphoreInfoCount = 1;

                VkSemaphoreSubmitInfo SemaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                SemaphoreInfo.semaphore = m_ImageAvailabeSemaphores[m_CurrentLocalFrame];
                Info.pSignalSemaphoreInfos = &SemaphoreInfo;
                ResultVk = VK_CALL(Device, vkQueueSubmit2(CommandQueueVk->Get(), 1, &Info, VK_NULL_HANDLE));
                C_ASSERT_VK_SUCCEEDED(ResultVk);
            }
        }
        else
        {
            CASSERT(C_SUCCEEDED(Result));
        }
    }
    else
    {
        CASSERT(ResultVk == VkResult::VK_SUCCESS || ResultVk == VkResult::VK_SUBOPTIMAL_KHR);
    }

    if (m_ImagesInFlight[m_CurrentImageIndex] != VK_NULL_HANDLE)
    {
        VK_CALL(Device, vkWaitForFences(Device.DeviceVk, 1, &m_ImagesInFlight[m_CurrentImageIndex], VK_TRUE, UINT64_MAX));
    }

    m_ImagesInFlight[m_CurrentImageIndex] = m_InflightFences[m_CurrentLocalFrame];

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CWindowContextVulkan::Present()
{
    C_STATUS Result = CWindowContext::Present();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    // #todo_vk optimize (here just need submit fence and semaphores for frame's end
    if (CCommandQueueVulkan* CommandQueue = GetCommandQueueVk(CommandQueueType::Graphics))
    {
        auto CurrentFrame = GetCurrentLocalFrame();

        CommandQueue->SubmitVk(nullptr, 0, GetImageAvailableSemaphore(CurrentFrame),
            GetInflightFence(CurrentFrame), true);
    }

    // #todo_vk
    Array<VkSemaphore, 20> WaitSemaphores;
    uint32_t WaitSemaphoresCount = 0;

    const auto& Device = m_Backend->GetDeviceManager().GetDevice(m_DeviceHandle);

    auto* GraphicsQueue = Device.GetCommandQueue(CommandQueueType::Graphics);
    CASSERT(GraphicsQueue);

    for (uint32_t i = 0; i < GraphicsQueue->GetSubmittedCommandBufferCount(); ++i)
    {
        CCommandBufferVulkan* CommandBuffer = GraphicsQueue->GetSubmittedCommandBuffer(i);
        if (CommandBuffer && CommandBuffer->GetCompletedSemaphore() != VK_NULL_HANDLE)
        {
            WaitSemaphores[WaitSemaphoresCount] = CommandBuffer->GetCompletedSemaphore();
            WaitSemaphoresCount++;
        }
    }

    VkPresentInfoKHR PresentInfo{};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = WaitSemaphoresCount;
    PresentInfo.pWaitSemaphores = WaitSemaphores.data();

    VkSwapchainKHR Swapchains[] = { m_Swapchain };
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = Swapchains;
    PresentInfo.pImageIndices = &m_CurrentImageIndex;
    PresentInfo.pResults = nullptr; // optional

    auto* PresentQueue = Device.GetCommandQueue(CommandQueueType::Present);
    CASSERT(PresentQueue);

    VkResult ResultVk = VK_CALL(Device, vkQueuePresentKHR(PresentQueue->Get(), &PresentInfo));

    if (ResultVk == VK_ERROR_OUT_OF_DATE_KHR || ResultVk == VK_SUBOPTIMAL_KHR)
    {
        Result = RecreateSwapchain();
        if (Result == C_STATUS::C_STATUS_OK)
        {
            return C_STATUS::C_STATUS_OK;
        }
        else if (Result != C_STATUS::C_STATUS_INVALID_ARG)
        {
            CASSERT(C_SUCCEEDED(Result));
        }
    }
    else
    {
        C_ASSERT_VK_SUCCEEDED(ResultVk);
    }

    m_CurrentLocalFrame = (m_CurrentLocalFrame + 1) % m_Backend->GetRenderer()->GetFramesInFlightCount();

    return C_STATUS::C_STATUS_OK;
}

void CWindowContextVulkan::DestroySyncObjects(bool IsDeferred)
{
    for (size_t i = 0; i < MAX_SWAPCHAIN_IMAGE_COUNT; ++i)
    {
        auto Destroy = [Backend = m_Backend, DeviceHandle = m_DeviceHandle, Surface = m_Surface,
            AvailSem = m_ImageAvailabeSemaphores[i], FinishedSem = m_RenderFinishedSemaphores[i], Fence = m_InflightFences[i]]()
        {
            const auto& Device = Backend->GetDeviceManager().GetDevice(DeviceHandle);

            if (AvailSem)
                VK_CALL(Device, vkDestroySemaphore(Device.DeviceVk, AvailSem, nullptr));

            if (FinishedSem)
                VK_CALL(Device, vkDestroySemaphore(Device.DeviceVk, FinishedSem, nullptr));

            if (Fence)
                VK_CALL(Device, vkDestroyFence(Device.DeviceVk, Fence, nullptr));

        };

        if (IsDeferred)
            m_Backend->GetDisposalManagerVk(m_DeviceHandle)->AddDisposable(Destroy);
        else
            Destroy();

        m_ImageAvailabeSemaphores[i] = nullptr;
        m_RenderFinishedSemaphores[i] = nullptr;
        m_InflightFences[i] = nullptr;
        m_ImagesInFlight[i] = nullptr;
    }
}

CCommandQueue* CWindowContextVulkan::GetCommandQueue(CommandQueueType QueueType) const
{
    return m_CommandQueueCache[(uint32)QueueType];
}

} // namespace Cyclone::Render
