#include "DefaultApplication.h"

#include "Engine/Framework/IPlatform.h"
#include "Engine/Framework/IRendererFactory.h"
#include "Engine/Framework/IRenderer.h"
#include "Engine/Framework/ISceneRenderer.h"
#include "Engine/Framework/IInputHandler.h"
#include "Engine/Framework/IUIModule.h"

#include "Engine/Framework/Impl/DefaultInputManager.h"

namespace Cyclone
{

DefaultApplication::DefaultApplication()
    : m_IsInit(false)
    , m_Dt(0.0)
    , m_Platform(nullptr)
{
}

DefaultApplication::~DefaultApplication()
{
    DeInit();
}

C_STATUS DefaultApplication::Init(const DefaultApplicationParams& Desc)
{
    m_IsInit = true;

    m_Platform = Desc.Platform;
    m_Renderer = Desc.Renderer;
    m_InputManager = Desc.InputManager;
    m_UI = Desc.UIModule;

    if (m_Platform == nullptr)
    {
        CASSERT(m_Platform);
        return C_STATUS::C_STATUS_INVALID_ARG;
    }

    // Executable runs from from <RootDir>/Bin to load binary dependencies (.dlls) from that folder
    // After that, need to change directory to <RootDir> to be able to reference assets and other project's files
    m_Platform->ChangeWorkingDirectory("..");

    C_ASSERT_RETURN_VAL(Desc.WindowsCount >= 1, C_STATUS::C_STATUS_INVALID_ARG);

    m_Windows.resize(Desc.WindowsCount);
    for (uint32 i = 0; i < Desc.WindowsCount; ++i)
    {
        m_Windows[i] = m_Platform->CreateWindowPtr();

        if (!m_Windows[i])
            return C_STATUS::C_STATUS_INVALID_ARG;
    }

    if (m_Renderer == nullptr || m_Windows[0] == nullptr)
        return C_STATUS::C_STATUS_INVALID_ARG;

    // #todo parse command line

    if (m_InputManager)
    {
        C_STATUS Result = m_InputManager->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    // initialize window
    for (uint32 i = 0; i < m_Windows.size(); ++i)
    {
        auto Window = m_Windows[i];
        if (Window)
        {
            WindowParams WindowParams{};
            WindowParams.App = this;
            WindowParams.Width = 1920;
            WindowParams.Height = 1080;
            WindowParams.Title = Desc.WindowCaption.empty() ? "Cyclone's Sample" : Desc.WindowCaption;
            if (i != 0)
                WindowParams.Title += " " + std::to_string(i);

            C_STATUS Result = Window->Init(&WindowParams);
            C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
        }
    }

    if (m_Renderer)
    {
        RendererDesc RendererDesc = RendererDesc
            .SetApplication(this)
            .SetWindow(m_Windows[0].get())
            .SetFrameCount(2);

        C_STATUS Result = m_Renderer->Init(&RendererDesc);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    if (m_UI)
    {
        C_STATUS Result = m_UI->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    {
        C_STATUS Result = OnInit();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    return C_STATUS::C_STATUS_OK;
}

void DefaultApplication::DeInit()
{
    if (m_IsInit == false)
        return;

    WaitAllPendingJobs();

    if (m_UI)
    {
        m_UI->Shutdown();
        m_UI = nullptr;
    }

    if (m_Renderer)
    {
        m_Renderer->Deinit();
        m_Renderer.reset();
    }

    for (uint32 i = 0; i < m_Windows.size(); ++i)
    {
        if (auto& Window = m_Windows[i])
        {
            Window.reset();
        }
    }
    m_Windows.clear();

    m_Platform = nullptr;

    m_IsInit = false;
}

int DefaultApplication::Run()
{
    struct HQTimeSample
    {
        void Start() {};
        void Stop() {};
        double Seconds() { return 0; }
    };

    HQTimeSample Timestamp;
    Timestamp.Start();

    bool NeedToExit = false;
    while (!NeedToExit)
    {
        Timestamp.Stop();
        m_Dt = Timestamp.Seconds();
        Timestamp.Start();

        NeedToExit = Frame() != C_STATUS::C_STATUS_OK;
    }

    return 0;
}

IInputHandler* DefaultApplication::GetInputHandler()
{
    return m_InputManager.get();
}

IInputManager* DefaultApplication::GetInputManager()
{
    return m_InputManager.get();
}

double DefaultApplication::GetDeltaTime() const
{
    return m_Dt;
}

void DefaultApplication::WaitAllPendingJobs()
{
    if (m_Renderer)
        m_Renderer->WaitGPU();
}

C_STATUS DefaultApplication::Frame()
{
    C_STATUS Result = BeginFrame();
    if (Result == C_STATUS::C_STATUS_SHOULD_EXIT)
        return Result;
    else
    {
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    Result = Update();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = Render();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = EndFrame();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS DefaultApplication::BeginFrame()
{
    // update window's message queues
    for (uint32 i = 0; i < m_Windows.size(); ++i)
    {
        if (auto Window = m_Windows[i])
        {
            C_STATUS Result = Window->UpdateMessageQueue();
            if (Result == C_STATUS::C_STATUS_SHOULD_EXIT)
                return C_STATUS::C_STATUS_SHOULD_EXIT;
            C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
        }
    }

    if (m_Renderer)
    {
        C_STATUS Result = m_Renderer->BeginFrame();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS DefaultApplication::Update()
{
    for (uint32 i = 0; i < m_Windows.size(); ++i)
    {
        if (auto Window = m_Windows[i])
        {
            Window->OnUpdate();
        }
    }

    if (m_InputManager)
    {
        m_InputManager->OnFrame();
    }

    if (m_UI)
    {
        m_UI->OnFrame();
    }

    // Update Callback
    {
        C_STATUS Result = OnUpdate();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    // Update UI Callback
    {
        C_STATUS Result = OnUpdateUI();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    for (uint32 i = 0; i < m_Windows.size(); ++i)
    {
        if (auto Window = m_Windows[i])
        {
            Window->OnUpdateAfter();
        }
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS DefaultApplication::Render()
{
    if (m_Renderer)
    {
        C_STATUS Result = m_Renderer->BeginRender();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

        Result = m_Renderer->Render();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

        if (m_UI)
        {
            Result = m_UI->OnRender();
            C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
        }

        Result = m_Renderer->EndRender();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS DefaultApplication::EndFrame()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS DefaultApplication::OnInit()
{
    // do nothing
    return C_STATUS::C_STATUS_OK;
}

C_STATUS DefaultApplication::OnUpdate()
{
    // do nothing
    return C_STATUS::C_STATUS_OK;
}

C_STATUS DefaultApplication::OnUpdateUI()
{
    // do nothing
    return C_STATUS::C_STATUS_OK;
}

C_STATUS DefaultApplication::OnRender()
{
    // do nothing
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
