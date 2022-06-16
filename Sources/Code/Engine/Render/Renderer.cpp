#include "Renderer.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IUISubsystem.h"

#include "IRendererBackend.h"
#include "SceneRenderer.h"

#include "Types/CommandQueue.h"
#include "Types/CommandBuffer.h"
#include "Types/WindowContext.h"
#include "Types/Texture.h"

namespace Cyclone::Render
{

Renderer::Renderer() = default;
Renderer::~Renderer()
{
    CASSERT(m_RendererBackend == nullptr);
}

C_STATUS Renderer::Init(const RendererDesc* Desc)
{
    CASSERT(m_RendererBackend);

    m_App = Desc->App;
    m_Windows = Desc->Windows;

    if (m_RendererBackend)
    {
        C_STATUS Result = m_RendererBackend->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    m_SceneRenderer = MakeUnique<CSceneRenderer>();
    C_STATUS Result = m_SceneRenderer->Init(this);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    for (uint32 i = 0; i < m_Windows.size(); ++i)
    {
        if (auto Window = m_Windows[i])
        {
            OnAddWindow(Window);
        }
    }

    return C_STATUS::C_STATUS_OK;
}

void Renderer::Deinit()
{
    //while (!m_WindowContexts.empty())
    //{
    //    OnRemoveWindow(m_WindowContexts.back().get());
    //}

    for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
    {
        if (auto& WindowContext = m_WindowContexts[i])
            WindowContext->Shutdown();
    }
    m_WindowContexts.clear();

    if (m_SceneRenderer)
    {
        m_SceneRenderer->DeInit();
    }
    m_SceneRenderer.reset();

    if (m_RendererBackend)
    {
        m_RendererBackend->Shutdown();
        m_RendererBackend = nullptr;
    }
}

C_STATUS Renderer::BeginFrame()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS Renderer::EndFrame()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS Renderer::BeginRender()
{
    if (m_RendererBackend)
    {
        C_STATUS Result = m_RendererBackend->BeginRender();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
    {
        C_STATUS Result = m_WindowContexts[i]->BeginRender();
        if (!C_SUCCEEDED(Result))
            return Result;
    }

    if (m_SceneRenderer)
    {
        C_STATUS Result = m_SceneRenderer->BeginRender();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS Renderer::Render()
{
    if (m_SceneRenderer)
    {
        C_STATUS Result = m_SceneRenderer->Render();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    if (IUISubsystem* UI = m_App->GetUI())
    {
        // #todo_ui refactor
        uint32 WindowsCount = std::min(1u, (uint32)m_WindowContexts.size());

        for (uint32 i = 0; i < WindowsCount; ++i)
        {
            CWindowContext* WindowContext = m_WindowContexts[i].get();
            CASSERT(WindowContext);
            CCommandQueue* CommandQueue = WindowContext->GetCommandQueue(CommandQueueType::Graphics);
            CASSERT(CommandQueue);
            CCommandBuffer* CommandBuffer = CommandQueue->AllocateCommandBuffer();
            CASSERT(CommandBuffer);

            //PROFILE_GPU_SCOPED_EVENT(CommandBuffer->Get(), "ImGUI Render");

            CommandBuffer->Begin();

            CRenderPass RenderPass{};
            RenderPass.RenderTargetSet.RenderTargetsCount = 1;
            RenderPass.RenderTargetSet.RenderTargets[0].RenderTarget = WindowContext->GetCurrentBackBuffer();
            RenderPass.RenderTargetSet.RenderTargets[0].InitialLayout = EImageLayoutType::ColorAttachment;
            RenderPass.RenderTargetSet.RenderTargets[0].Layout = EImageLayoutType::ColorAttachment;
            RenderPass.RenderTargetSet.RenderTargets[0].FinalLayout = EImageLayoutType::Present;
            RenderPass.RenderTargetSet.RenderTargets[0].LoadOp = ERenderTargetLoadOp::Load;
            RenderPass.RenderTargetSet.RenderTargets[0].StoreOp = ERenderTargetStoreOp::Store;

            RenderPass.ViewportExtent = { 0, 0, 
                (float)WindowContext->GetCurrentBackBuffer()->Texture->GetDesc().Width, 
                (float)WindowContext->GetCurrentBackBuffer()->Texture->GetDesc().Height 
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

C_STATUS Renderer::EndRender()
{
    if (m_SceneRenderer)
    {
        C_STATUS Result = m_SceneRenderer->EndRender();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    // Present
    for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
    {
        C_STATUS Result = m_WindowContexts[i]->Present();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    if (m_RendererBackend)
    {
        C_STATUS result = m_RendererBackend->EndRender();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    m_CurrentFrame++;
    m_CurrentLocalFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return C_STATUS::C_STATUS_OK;
}

void Renderer::WaitGPU()
{
    m_RendererBackend->WaitGPU();
}

C_STATUS Renderer::PreInit(IRendererBackend* RendererBackend)
{
    m_RendererBackend = RendererBackend;
    return C_STATUS::C_STATUS_OK;
}

CWindowContext* Renderer::GetWindowContext(IWindow* Window)
{
    for (auto& Context : m_WindowContexts)
    {
        if (Context->GetWindow() == Window)
            return Context.get();
    }
    return nullptr;
}

CWindowContext* Renderer::GetDefaultWindowContext()
{
    if (m_WindowContexts.size() > 0)
        return m_WindowContexts[0].get();

    return nullptr;
}

CCommandQueue* Renderer::GetDefaultCommandQueue(CommandQueueType Type)
{
    if (CWindowContext* WindowContext = GetDefaultWindowContext())
        return WindowContext->GetCommandQueue(Type);

    return nullptr;
}

CWindowContext* Renderer::OnAddWindow(IWindow* Window)
{
    if (Window == nullptr)
        return nullptr;

#if _DEBUG
    CASSERT(GetWindowContext(Window) == nullptr);
#endif
    
    m_WindowContexts.emplace_back(m_RendererBackend->CreateWindowContext(Window));
    C_ASSERT_RETURN_VAL(m_WindowContexts.back(), nullptr);

    return nullptr;
}

void Renderer::OnRemoveWindow(IWindow* Window)
{
#if _DEBUG
    CASSERT(GetWindowContext(Window) != nullptr);
#endif

    // #todo_vk remove scene views for each of this window?

    for(auto it = m_WindowContexts.begin(); it != m_WindowContexts.end(); ++it)
    {
        if ((*it)->GetWindow() == Window)
        {
            m_WindowContexts.erase(it);
            return;
        }
    }
}

CSceneRenderer* Renderer::GetSceneRenderer()
{
    return m_SceneRenderer.get();
}

CRasterizerState* Renderer::GetRasterizerState(RasterizerState State)
{
    // #todo_vk
    static CRasterizerState RState{};
    return &RState;
}

CBlendState* Renderer::GetBlendState(BlendState State)
{
    static CBlendState BState{};
    return &BState;
}

CDepthStencilState* Renderer::GetDepthStencilState(DepthStencilState State)
{
    static CDepthStencilState DState{};
    return &DState;
}

} // namespace Cyclone::Render
