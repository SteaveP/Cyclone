#include "RenderSceneSubsystem.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IRenderer.h"

#include "Engine/Scene/SceneSubsystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneViewport.h"

#include "Engine/Render/Scene/RenderScene.h"
#include "Engine/Render/Scene/RenderSceneView.h"
#include "Engine/Render/Scene/SceneRenderer.h"

#include "Engine/Render/Backend/IRendererBackend.h"
#include "Engine/Render/Backend/IResourceManager.h"
#include "Engine/Render/Backend/WindowContext.h"
#include "Engine/Render/Backend/Resource.h"
#include "Engine/Render/Backend/ResourceView.h"

namespace Cyclone::Render
{

CRenderSceneSubsystem::CRenderSceneSubsystem() : ISubsystem("RenderSceneSubsystem") {}
CRenderSceneSubsystem::CRenderSceneSubsystem(CRenderSceneSubsystem&& Other) noexcept = default;
CRenderSceneSubsystem& CRenderSceneSubsystem::operator =(CRenderSceneSubsystem&& Other) noexcept = default;
CRenderSceneSubsystem::~CRenderSceneSubsystem()
{
    DeInitImpl();
}

C_STATUS CRenderSceneSubsystem::Init(IApplication* App)
{
    m_App = App;
    m_Renderer = m_App->GetRenderer();
    CASSERT(m_App);

    m_SceneRenderer = MakeUnique<CSceneRenderer>();
    C_STATUS Result = m_SceneRenderer->Init(this, m_Renderer);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

void CRenderSceneSubsystem::DeInit()
{
    DeInitImpl();
}

void CRenderSceneSubsystem::DeInitImpl() noexcept
{
    m_SceneRenderer.reset();
    m_RenderScenes.clear();
}

C_STATUS CRenderSceneSubsystem::OnRegister()
{
    m_SceneSys = (CSceneSubsystem*)m_App->GetSubsystem("SceneSubsystem");
    CASSERT(m_SceneSys);

    m_SceneSys->OnSceneAddedDelegate += DelegateLib::DelegateMember1(this, &CRenderSceneSubsystem::OnSceneAdded);
    m_SceneSys->OnSceneRemovedDelegate += DelegateLib::DelegateMember1(this, &CRenderSceneSubsystem::OnSceneRemoved);

    m_SceneSys->OnSceneViewportAddedDelegate += DelegateLib::DelegateMember1(this, &CRenderSceneSubsystem::OnSceneViewAdded);
    m_SceneSys->OnSceneViewportRemovedDelegate += DelegateLib::DelegateMember1(this, &CRenderSceneSubsystem::OnSceneViewRemoved);

    *m_Renderer->GetOnBeginRenderDelegate() += DelegateLib::DelegateMember0(this, &CRenderSceneSubsystem::OnBeginRender);
    *m_Renderer->GetOnRenderDelegate() += DelegateLib::DelegateMember0(this, &CRenderSceneSubsystem::OnRender);
    *m_Renderer->GetOnEndRenderDelegate() += DelegateLib::DelegateMember0(this, &CRenderSceneSubsystem::OnEndRender);

    return C_STATUS::C_STATUS_OK;
}

void CRenderSceneSubsystem::OnUnRegister()
{
    if (m_SceneSys)
    {
        m_SceneSys->OnSceneAddedDelegate -= DelegateLib::DelegateMember1(this, &CRenderSceneSubsystem::OnSceneAdded);
        m_SceneSys->OnSceneRemovedDelegate -= DelegateLib::DelegateMember1(this, &CRenderSceneSubsystem::OnSceneRemoved);

        m_SceneSys->OnSceneViewportAddedDelegate -= DelegateLib::DelegateMember1(this, &CRenderSceneSubsystem::OnSceneViewAdded);
        m_SceneSys->OnSceneViewportRemovedDelegate -= DelegateLib::DelegateMember1(this, &CRenderSceneSubsystem::OnSceneViewRemoved);
    }

    if (m_Renderer)
    {
        *m_Renderer->GetOnBeginRenderDelegate() -= DelegateLib::DelegateMember0(this, &CRenderSceneSubsystem::OnBeginRender);
        *m_Renderer->GetOnRenderDelegate() -= DelegateLib::DelegateMember0(this, &CRenderSceneSubsystem::OnRender);
        *m_Renderer->GetOnEndRenderDelegate() -= DelegateLib::DelegateMember0(this, &CRenderSceneSubsystem::OnEndRender);
    }

    m_SceneSys = nullptr;
    m_App = nullptr;
    m_Renderer = nullptr;

    CASSERT(m_RenderScenes.empty());
    m_RenderScenes.clear();
}

void CRenderSceneSubsystem::OnSceneAdded(Ptr<CScene> Scene)
{
    // #todo_mt add multithreading assert that called not from render threads

    CASSERT(GetRenderScene(Scene.get()) == nullptr);

    auto& RenderScene = m_RenderScenes.emplace_back(MakeShared<CRenderScene>());
    RenderScene->m_Scene = Scene.get();

    if (OnRenderSceneAddedDelegate)
        OnRenderSceneAddedDelegate(RenderScene);
}

void CRenderSceneSubsystem::OnSceneRemoved(Ptr<CScene> Scene)
{
    // #todo_mt add multithreading assert that called not from render threads

    Ptr<CRenderScene> RenderScene = GetRenderScene(Scene.get());

    if (OnRenderSceneRemovedDelegate)
        OnRenderSceneRemovedDelegate(RenderScene);

    for (auto it = m_RenderScenes.begin(); it != m_RenderScenes.end(); ++it)
    {
        if (*it = RenderScene)
        {
            m_RenderScenes.erase(it);
            break;
        }
    }
}

void CRenderSceneSubsystem::OnSceneViewAdded(Ptr<CSceneViewport> SceneView)
{
    // #todo_mt add multithreading assert that called not from render threads

    const auto& RenderScene = GetRenderScene(SceneView->Scene.get());
    auto RenderSceneView = MakeShared<CRenderSceneView>(); // #todo_vk refactor to handles?
    RenderSceneView->Viewport = SceneView.get();
    RenderSceneView->RenderScene = RenderScene.get();
    RenderSceneView->RenderWindowContext = m_Renderer->GetWindowContext(SceneView->Window.get());

    IResourceManager* ResourceManager = m_Renderer->GetRendererBackend()->GetResourceManager(RenderSceneView->RenderWindowContext->GetDeviceHandle());

    {
        CResourceDesc Desc{};
        Desc.Backend = m_Renderer->GetRendererBackend();
        Desc.DeviceHandle = RenderSceneView->RenderWindowContext->GetDeviceHandle();
        Desc.Flags = EResourceFlags::Texture;
        Desc.InitialLayout = EImageLayoutType::Undefined;
        Desc.Texture.InitialUsage = EImageUsageType::None;
        Desc.Texture.Usage = EImageUsageType::ColorAttachment | EImageUsageType::ShaderResourceView;

        auto& BackBufferDesc = ResourceManager->GetResource(RenderSceneView->RenderWindowContext->GetBackBuffer(0).Texture)->GetDesc();
        Desc.Format = BackBufferDesc.Format;
        Desc.Texture.Width = BackBufferDesc.Texture.Width;
        Desc.Texture.Height = BackBufferDesc.Texture.Height;

#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.Name = ("RenderSceneView" + ToString(RenderScene->m_Views.size()));
#endif
        RenderSceneView->RenderTarget.Texture = ResourceManager->CreateResource(Desc);
        C_ASSERT_RETURN(RenderSceneView->RenderTarget.Texture.IsValid());

        CResourceViewDesc ViewDesc{};
        ViewDesc.Backend = m_Renderer->GetRendererBackend();
        ViewDesc.Type = EResourceFlags::Texture;
        ViewDesc.Resource = RenderSceneView->RenderTarget.Texture;
        ViewDesc.Format = Desc.Format;
        ViewDesc.Texture.ViewType = ETextureViewType::Type2D;
        ViewDesc.Texture.AspectMask = EImageAspectType::Color;
#if ENABLE_DEBUG_RENDER_BACKEND
        ViewDesc.Name = Desc.Name + " RTV";
#endif
        RenderSceneView->RenderTarget.RenderTargetView = ResourceManager->CreateResourceView(ViewDesc);
        C_ASSERT_RETURN(RenderSceneView->RenderTarget.RenderTargetView.IsValid());

#if ENABLE_DEBUG_RENDER_BACKEND
        ViewDesc.Name = Desc.Name + " SRV";
#endif
        RenderSceneView->RenderTarget.ShaderResourceView = ResourceManager->CreateResourceView(ViewDesc);
        C_ASSERT_RETURN(RenderSceneView->RenderTarget.ShaderResourceView.IsValid());
    }

    C_STATUS Result = RenderScene->AddSceneView(RenderSceneView);
    C_ASSERT_RETURN(C_SUCCEEDED(Result));

    if (OnRenderSceneViewAddedDelegate)
        OnRenderSceneViewAddedDelegate(RenderSceneView);
}

void CRenderSceneSubsystem::OnSceneViewRemoved(Ptr<CSceneViewport> SceneView)
{
    // #todo_mt add multithreading assert that called not from render threads

    Ptr<CRenderSceneView> View = GetRenderSceneView(SceneView.get());

    if (OnRenderSceneViewRemovedDelegate)
        OnRenderSceneViewRemovedDelegate(View);

    IResourceManager* ResourceManager = m_Renderer->GetRendererBackend()->GetResourceManager(View->RenderWindowContext->GetDeviceHandle());

    if (View->RenderTarget.RenderTargetView.IsValid())
        ResourceManager->DestroyResourceView(View->RenderTarget.RenderTargetView);
    if (View->RenderTarget.ShaderResourceView.IsValid())
        ResourceManager->DestroyResourceView(View->RenderTarget.ShaderResourceView);
    if (View->RenderTarget.DepthStencilView.IsValid())
        ResourceManager->DestroyResourceView(View->RenderTarget.DepthStencilView);
    if (View->RenderTarget.UnorderedAccessView.IsValid())
        ResourceManager->DestroyResourceView(View->RenderTarget.UnorderedAccessView);

    if (View->RenderTarget.Texture.IsValid())
        ResourceManager->DestroyResource(View->RenderTarget.Texture);

    View->RenderScene->RemoveSceneView(View.get());
}

Ptr<CRenderScene> CRenderSceneSubsystem::GetRenderScene(CScene* Scene)
{
    for (uint32 s = 0; s < m_RenderScenes.size(); ++s)
    {
        if (m_RenderScenes[s] && m_RenderScenes[s]->m_Scene == Scene)
            return m_RenderScenes[s];
    }

    return nullptr;
}

Ptr<CRenderSceneView> CRenderSceneSubsystem::GetRenderSceneView(CSceneViewport* Viewport)
{
    CRenderScene* RenderScene = GetRenderScene(Viewport->Scene.get()).get();
    if (RenderScene == nullptr)
        return nullptr;

    for (uint32 v = 0; v < (uint32)RenderScene->m_Views.size(); ++v)
    {
        if (RenderScene->m_Views[v]->Viewport == Viewport)
            return RenderScene->m_Views[v];
    }

    return nullptr;
}

void CRenderSceneSubsystem::OnBeginRender()
{
    m_SceneRenderer->BeginRender();
}

void CRenderSceneSubsystem::OnRender()
{
    m_SceneRenderer->Render();
}

void CRenderSceneSubsystem::OnEndRender()
{
    m_SceneRenderer->EndRender();
}

} // namespace Cyclone::Render
