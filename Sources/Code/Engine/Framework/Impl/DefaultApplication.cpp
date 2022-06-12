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
    : m_isInit(false)
    , m_dt(0.0)
    , m_platform(nullptr)
{
}

DefaultApplication::~DefaultApplication()
{
    DeInit();
}

C_STATUS DefaultApplication::Init(const DefaultApplicationParams& desc)
{
    m_isInit = true;

    m_platform = desc.Platform;
    m_renderer = desc.Renderer;
    m_inputManager = desc.InputManager;
    m_ui = desc.UIModule;

    if (m_platform == nullptr)
    {
        CASSERT(m_platform);
        return C_STATUS::C_STATUS_INVALID_ARG;
    }

    // Executable runs from from <RootDir>/Bin to load binary dependencies (.dlls) from that folder
    // After that, need to change directory to <RootDir> to be able to reference assets and other project's files
    m_platform->ChangeWorkingDirectory("..");

    m_window = m_platform->CreateWindowPtr();

    if (m_renderer == nullptr || m_window == nullptr)
        return C_STATUS::C_STATUS_INVALID_ARG;

    // #todo parse command line

    if (m_inputManager)
    {
        C_STATUS Result = m_inputManager->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    // initialize window
    if (m_window)
    {
        WindowParams WindowParams{};
        WindowParams.app = this;
        WindowParams.width = 1920;
        WindowParams.height = 1080;
        WindowParams.title = desc.WindowCaption.empty() ? "Cyclone's Sample" : desc.WindowCaption;

        C_STATUS Result = m_window->Init(&WindowParams);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    if (m_renderer)
    {
        RendererDesc rendererDesc = RendererDesc()
            .SetApplication(this)
            .SetWindow(m_window.get())
            .SetFrameCount(2);

        C_STATUS Result = m_renderer->Init(&rendererDesc);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    if (m_ui)
    {
        C_STATUS Result = m_ui->Init(this);
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
    if (m_isInit == false)
        return;

    WaitAllPendingJobs();

    if (m_ui)
    {
        m_ui->Shutdown();
        m_ui = nullptr;
    }

    if (m_renderer)
    {
        m_renderer->Deinit();
        m_renderer.reset();
    }

    if (m_window)
    {
        m_window.reset();
    }

    m_platform = nullptr;

    m_isInit = false;
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
        m_dt = Timestamp.Seconds();
        Timestamp.Start();

        NeedToExit = Frame() != C_STATUS::C_STATUS_OK;
    }

    return 0;
}

IInputHandler* DefaultApplication::GetInputHandler()
{
    return m_inputManager.get();
}

IInputManager* DefaultApplication::GetInputManager()
{
    return m_inputManager.get();
}

double DefaultApplication::GetDeltaTime() const
{
    return m_dt;
}

void DefaultApplication::WaitAllPendingJobs()
{
    if (m_renderer)
        m_renderer->WaitGPU();
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
    if (m_window)
    {
        C_STATUS Result = m_window->UpdateMessageQueue();
        if (Result == C_STATUS::C_STATUS_SHOULD_EXIT)
            return C_STATUS::C_STATUS_SHOULD_EXIT;
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    if (m_renderer)
    {
        C_STATUS Result = m_renderer->BeginFrame();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS DefaultApplication::Update()
{
    if (m_window)
    {
        m_window->OnUpdate();
    }

    if (m_inputManager)
    {
        m_inputManager->OnFrame();
    }

    if (m_ui)
    {
        m_ui->OnFrame();
    }

    // Update Callback
    {
        C_STATUS result = OnUpdate();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    // Update UI Callback
    {
        C_STATUS result = OnUpdateUI();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    if (m_window)
    {
        m_window->OnUpdateAfter();
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS DefaultApplication::Render()
{
    if (m_renderer)
    {
        C_STATUS Result = m_renderer->BeginRender();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

        Result = m_renderer->Render();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

        if (m_ui)
        {
            Result = m_ui->OnRender();
            C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
        }

        Result = m_renderer->EndRender();
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
