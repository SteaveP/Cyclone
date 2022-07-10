#include "Renderer.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IUISubsystem.h"
#include "Engine/Framework/IPlatform.h"
#include "Engine/Platform/IEvent.h"
#include "Engine/Utils/Profiling.h"
#include "Engine/Utils/Delegate.h"
#include "Engine/Utils/Log.h"

#include "Backend/IRendererBackend.h"
#include "Backend/IResourceManager.h"
#include "Backend/CommandQueue.h"
#include "Backend/CommandBuffer.h"
#include "Backend/WindowContext.h"
#include "Backend/Resource.h"

namespace Cyclone::Render
{

struct CThreadContext
{
    Ptr<IEvent> OnFrameWaitRT;
    Ptr<IEvent> OnFrameSignalRT;

    Ptr<IEvent> OnFrameUIWaitRT;
    bool ShouldExit = false;
};
class CRenderThread
{
public:
    static void Main(RawPtr ThreadContext);
    
    IApplication* m_App = nullptr;
    IRenderer* m_Renderer = nullptr;

    CThreadContext m_Context;
};

void CRenderThread::Main(RawPtr ThreadContextRaw)
{
    OPTICK_THREAD("RenderThread");

    CRenderThread* Context = reinterpret_cast<CRenderThread*>(ThreadContextRaw);
    CASSERT(Context);

    bool ShouldExit = false;
    while (!ShouldExit)
    {
        LOG_TRACE("RenderThread: Wait BeginFrame");
        Context->m_Context.OnFrameWaitRT->Wait();
        LOG_TRACE("RenderThread: Start Frame");

        if (Context->m_Context.ShouldExit)
        {
            ShouldExit = true;
            Context->m_Context.OnFrameSignalRT->Signal();
            break;
        }

        // #todo_mt
        // Process tasks

        // Call render function
        C_STATUS Result = Context->m_Renderer->RenderFrame();
        CASSERT(C_SUCCEEDED(Result));

        // Process any pending tasks

        if (Result == C_STATUS::C_STATUS_SHOULD_EXIT)
            ShouldExit = true;

        LOG_TRACE("RenderThread: Signal EndFrame");
        Context->m_Context.OnFrameSignalRT->Signal();
    }

    // #todo_mt Check that no pending tasks or process them
}

CRenderThread GRenderThread;

CRenderer::CRenderer() = default;
CRenderer::CRenderer(CRenderer&& Other) noexcept = default;
CRenderer& CRenderer::operator=(CRenderer&& Other) noexcept = default;
CRenderer::~CRenderer()
{
    DeInitImpl();
    CASSERT(m_RendererBackend == nullptr);
}

C_STATUS CRenderer::Init(const CRendererDesc* Desc)
{
    CASSERT(m_RendererBackend);

    m_App = Desc->App;
    m_FramesInFlightCount = Desc->FramesInFlightCount;
    m_EnableMultithreading = Desc->EnableMultiThreading;

    CASSERT(Desc->FramesInFlightCount <= MAX_FRAMES_IN_FLIGHT);

    if (m_RendererBackend)
    {
        C_STATUS Result = m_RendererBackend->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    if (m_EnableMultithreading)
    {
        GRenderThread.m_Context.OnFrameSignalRT = m_App->GetPlatform()->CreateEventPtr();
        EEventFlags Flags = EEventFlags::AutoReset;
        C_STATUS Result = GRenderThread.m_Context.OnFrameSignalRT->Init("OnFrameSignalRT", Flags);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

        GRenderThread.m_Context.OnFrameWaitRT = m_App->GetPlatform()->CreateEventPtr();
        Flags = EEventFlags::AutoReset;
        Result = GRenderThread.m_Context.OnFrameWaitRT->Init("OnFrameWaitRT", Flags);

        GRenderThread.m_Context.OnFrameUIWaitRT = m_App->GetPlatform()->CreateEventPtr();
        Flags = EEventFlags::AutoReset;
        Result = GRenderThread.m_Context.OnFrameUIWaitRT->Init("OnFrameUIWaitRT", Flags);

        GRenderThread.m_App = m_App;
        GRenderThread.m_Renderer = this;

        m_RenderThread = MakeUnique<std::thread>(CRenderThread::Main, &GRenderThread);

        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    RasterizerDefault = CreateRasterizerState(CRasterizerState{});
    BlendDisabled = CreateBlendState(CBlendState{});
    DepthStencilDisabled = CreateDepthStencilState(CDepthStencilState{});
    DepthStencilDepthRead = CreateDepthStencilState(CDepthStencilState{ .DepthTestEnable = true });
    DepthStencilDepthWrite= CreateDepthStencilState(CDepthStencilState{ .DepthTestEnable = true, .DepthWriteEnable = true});

    *m_App->GetOnWindowAddedDelegate() += DelegateLib::DelegateMember1(this, &CRenderer::OnWindowAdded);
    *m_App->GetOnWindowRemovedDelegate() += DelegateLib::DelegateMember1(this, &CRenderer::OnWindowRemoved);

    return C_STATUS::C_STATUS_OK;
}

void CRenderer::DeInit()
{
    DeInitImpl();
}

void CRenderer::DeInitImpl() noexcept
{
    if (m_EnableMultithreading)
    {
        if (m_RenderThread)
        {
            GRenderThread.m_Context.ShouldExit = true;
            GRenderThread.m_Context.OnFrameWaitRT->Signal();

            m_RenderThread->join();
        }

        m_RenderThread.reset();
    }

    if (m_App)
    {
        *m_App->GetOnWindowAddedDelegate() -= DelegateLib::DelegateMember1(this, &CRenderer::OnWindowAdded);
        *m_App->GetOnWindowRemovedDelegate() -= DelegateLib::DelegateMember1(this, &CRenderer::OnWindowRemoved);
    }

    CASSERT(m_WindowContexts.empty());

    // Additional checks & deinits
    if (m_RendererBackend)
    {
        for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
        {
            if (m_WindowContexts[i].IsValid())
                m_RendererBackend->DestroyWindowContext(m_WindowContexts[i]);
        }
        m_WindowContexts.clear();

        m_RendererBackend->Shutdown();
        m_RendererBackend = nullptr;
    }

    if (RasterizerDefault.IsValid())
        DestroyRasterizerState(std::exchange(RasterizerDefault, {}));
    if (BlendDisabled.IsValid())
        DestroyBlendState(std::exchange(BlendDisabled, {}));
    if (DepthStencilDisabled.IsValid())
        DestroyDepthStencilState(std::exchange(DepthStencilDisabled, {}));
    if (DepthStencilDepthRead.IsValid())
        DestroyDepthStencilState(std::exchange(DepthStencilDepthRead, {}));
    if (DepthStencilDepthWrite.IsValid())
        DestroyDepthStencilState(std::exchange(DepthStencilDepthWrite, {}));
}

C_STATUS CRenderer::BeginRender()
{
    if (m_RendererBackend)
    {
        C_STATUS Result = m_RendererBackend->BeginRender();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

        for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
        {
            CWindowContext* WindowContext = m_RendererBackend->GetWindowContext(m_WindowContexts[i]);
            C_STATUS Result = WindowContext->BeginRender();
            if (!C_SUCCEEDED(Result))
                return Result;
        }
    }

    if (m_OnBeginRenderDelegate)
    {
        m_OnBeginRenderDelegate();
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CRenderer::Render()
{
    OPTICK_CATEGORY("Render", Optick::Category::Rendering);

    CWindowContext* WindowContext = m_WindowContexts.size() > 0 ? m_RendererBackend->GetWindowContext(m_WindowContexts[0]) : nullptr;
    CCommandQueue* CommandQueue = WindowContext ? WindowContext->GetCommandQueue(CommandQueueType::Graphics) : nullptr;

    PROFILE_GPU_SCOPED_EVENT(CommandQueue, "Frame %d", GetCurrentFrame());

    if (m_OnRenderDelegate)
    {
        m_OnRenderDelegate();
    }

    if (IUISubsystem* UI = m_App->GetUI())
    {
        // #todo_vk_ui_first #todo_7 fixme crude hack, ImGUI needs to be parallelized
        if (m_EnableMultithreading)
        {
            GRenderThread.m_Context.OnFrameUIWaitRT->Wait();
        }

        PROFILE_GPU_SCOPED_EVENT(CommandQueue, "UI");

        // #todo_ui #todo_vk refactor
        uint32 WindowsCount = std::min(1u, (uint32)m_WindowContexts.size());
        for (uint32 i = 0; i < WindowsCount; ++i)
        {
            CWindowContext* WindowContext = m_RendererBackend->GetWindowContext(m_WindowContexts[i]);
            CASSERT(WindowContext);

            IResourceManager* ResourceManager = m_RendererBackend->GetResourceManager(WindowContext->GetDeviceHandle());
            CASSERT(ResourceManager);

            CCommandQueue* CommandQueue = WindowContext->GetCommandQueue(CommandQueueType::Graphics);
            CASSERT(CommandQueue);
            CCommandBuffer* CommandBuffer = CommandQueue->AllocateCommandBuffer();
            CASSERT(CommandBuffer);


            CommandBuffer->Begin();

            PROFILE_GPU_SCOPED_EVENT(CommandBuffer, "ImGUI %d", i);

            CRenderPass RenderPass{};
            RenderPass.RenderTargetSet.RenderTargetsCount = 1;
            RenderPass.RenderTargetSet.RenderTargets[0].RenderTarget = WindowContext->GetCurrentBackBuffer();
            RenderPass.RenderTargetSet.RenderTargets[0].FinalLayout = EImageLayoutType::Present;
            RenderPass.RenderTargetSet.RenderTargets[0].FinalUsage = EImageUsageType::Present;
            RenderPass.RenderTargetSet.RenderTargets[0].LoadOp = ERenderTargetLoadOp::DontCare;
            RenderPass.RenderTargetSet.RenderTargets[0].StoreOp = ERenderTargetStoreOp::Store;

            CResource* BackBufferTex = ResourceManager->GetResource(WindowContext->GetCurrentBackBuffer().Texture);
            CASSERT(BackBufferTex);

            RenderPass.ViewportExtent = { 0, 0, 
                (float)BackBufferTex->GetDesc().Texture.Width, 
                (float)BackBufferTex->GetDesc().Texture.Height 
            };

            CommandBuffer->BeginRenderPass(RenderPass);

            C_STATUS Result = UI->OnRender(CommandBuffer);
            C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

            CommandBuffer->EndRenderPass();
            CommandBuffer->End();

            CommandQueue->Submit(&CommandBuffer, 1, true);
        }
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CRenderer::EndRender()
{
    if (m_OnEndRenderDelegate)
    {
        m_OnEndRenderDelegate();
    }

    // Present
    for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
    {
        CWindowContext* WindowContext = m_RendererBackend->GetWindowContext(m_WindowContexts[i]);
        C_STATUS Result = WindowContext->Present();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    if (m_RendererBackend)
    {
        C_STATUS result = m_RendererBackend->EndRender();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    ++m_CurrentFrame;
    m_CurrentLocalFrame = m_CurrentFrame % m_FramesInFlightCount;

    return C_STATUS::C_STATUS_OK;
}

void CRenderer::WaitGPU()
{
    // #todo_vk flush all pending work?
    m_RendererBackend->WaitGPU();
}

C_STATUS CRenderer::PreInit(IRendererBackend* RendererBackend)
{
    m_RendererBackend = RendererBackend;
    return C_STATUS::C_STATUS_OK;
}

CWindowContext* CRenderer::GetWindowContext(IWindow* Window)
{
    for (auto& ContextHandle : m_WindowContexts)
    {
        CWindowContext* WindowContext = m_RendererBackend->GetWindowContext(ContextHandle);
        if (WindowContext->GetWindow() == Window)
            return WindowContext;
    }
    return nullptr;
}

CWindowContext* CRenderer::GetDefaultWindowContext()
{
    if (m_WindowContexts.size() > 0)
        return m_RendererBackend->GetWindowContext(m_WindowContexts[0]);

    return nullptr;
}

CCommandQueue* CRenderer::GetDefaultCommandQueue(CommandQueueType Type)
{
    if (CWindowContext* WindowContext = GetDefaultWindowContext())
        return WindowContext->GetCommandQueue(Type);

    return nullptr;
}

void CRenderer::OnWindowAdded(IWindow* Window)
{
    // #todo_vk_refactor #todo_mt add to queue
    if (Window == nullptr)
        return;

#if _DEBUG
    CASSERT(GetWindowContext(Window) == nullptr);
#endif
    
    auto& WindowContextHandle = m_WindowContexts.emplace_back(m_RendererBackend->CreateWindowContext());
    CASSERT(WindowContextHandle.IsValid());
    CWindowContext* WindowContext = m_RendererBackend->GetWindowContext(WindowContextHandle);

    C_STATUS Result = WindowContext->Init(this, Window);
    C_ASSERT_RETURN(C_SUCCEEDED(Result));

    if (m_OnWindowContextAddedDelegate)
        m_OnWindowContextAddedDelegate(WindowContextHandle);
}

void CRenderer::OnWindowRemoved(IWindow* Window)
{
    // #todo_vk_refactor #todo_mt add to queue
#if _DEBUG
    CASSERT(GetWindowContext(Window) != nullptr);
#endif

    for(auto it = m_WindowContexts.begin(); it != m_WindowContexts.end(); ++it)
    {
        CWindowContext* WindowContext = m_RendererBackend->GetWindowContext(*it);
        if (WindowContext->GetWindow() == Window)
        {
            if (m_OnWindowContextRemovedDelegate)
                m_OnWindowContextRemovedDelegate(*it);

            WindowContext->DeInit();
            m_WindowContexts.erase(it);
            return;
        }
    }
}

C_STATUS CRenderer::OnWindowResize(IWindow* Window)
{
    // #todo_vk_swapchain #todo_mt make proper multithreading support
    //CASSERT(that render thread is still waiting of the beginning of the frame);
    if (CWindowContext* Context = GetWindowContext(Window))
    {
        return Context->OnWindowResize();
    }

    return C_STATUS::C_STATUS_INVALID_ARG;
}

C_STATUS CRenderer::RenderFrame()
{
    C_STATUS Result = BeginRender();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = Render();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = EndRender();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CRenderer::EnqueueFrame()
{
    if (m_EnableMultithreading)
    {
        CASSERT(m_RenderThread);
        LOG_TRACE("Main: EnqueueFrame for Renderer");
        GRenderThread.m_Context.OnFrameWaitRT->Signal();
    }
    else
    {
        RenderFrame();
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CRenderer::EnqueueFrameUI()
{
    if (m_EnableMultithreading)
    {
        CASSERT(m_RenderThread);
        LOG_TRACE("Main: EnqueueFrameUI for Renderer");
        GRenderThread.m_Context.OnFrameUIWaitRT->Signal();
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CRenderer::WaitFrame(bool ShouldTerminate)
{
    if (m_EnableMultithreading)
    {
        CASSERT(m_RenderThread);
        if (ShouldTerminate)
        {
            GRenderThread.m_Context.OnFrameWaitRT->Signal(); // #todo_vk is this correct?
            GRenderThread.m_Context.ShouldExit = true;
        }

        LOG_TRACE("Main: Wait for Renderer Frame");
        GRenderThread.m_Context.OnFrameSignalRT->Wait();
    }
    else
    {
        // do nothing
    }

    return C_STATUS::C_STATUS_OK;
}

CHandle<CRasterizerState> CRenderer::CreateRasterizerState(CRasterizerState State)
{
    CHandle<CRasterizerState> Handle = m_RasterizerStatePool.Create();
    *m_RasterizerStatePool.Get(Handle) = State;
    return Handle;
}

CHandle<CBlendState> CRenderer::CreateBlendState(CBlendState State)
{
    CHandle<CBlendState> Handle = m_BlendStatePool.Create();
    *m_BlendStatePool.Get(Handle) = State;
    return Handle;
}

CHandle<CDepthStencilState> CRenderer::CreateDepthStencilState(CDepthStencilState State)
{
    CHandle<CDepthStencilState> Handle = m_DepthStencilStatePool.Create();
    *m_DepthStencilStatePool.Get(Handle) = State;
    return Handle;
}

void CRenderer::DestroyRasterizerState(CHandle<CRasterizerState> Handle)
{
    m_RasterizerStatePool.Destroy(Handle);
}

void CRenderer::DestroyBlendState(CHandle<CBlendState> Handle)
{
    m_BlendStatePool.Destroy(Handle);
}

void CRenderer::DestroyDepthStencilState(CHandle<CDepthStencilState> Handle)
{
    m_DepthStencilStatePool.Destroy(Handle);
}

const CRasterizerState* CRenderer::GetRasterizerState(CHandle<CRasterizerState> Handle) const
{
    return m_RasterizerStatePool.Get(Handle);
}

const CBlendState* CRenderer::GetBlendState(CHandle<CBlendState> Handle) const
{
    return m_BlendStatePool.Get(Handle);
}

const CDepthStencilState* CRenderer::GetDepthStencilState(CHandle<CDepthStencilState> Handle) const
{
    return m_DepthStencilStatePool.Get(Handle);
}

} // namespace Cyclone::Render
