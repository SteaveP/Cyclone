#include "DefaultApplication.h"

#include "Engine/Core/Types.h"

#include "Engine/Framework/IPlatform.h"
#include "Engine/Framework/IRenderer.h"
#include "Engine/Framework/IInputHandler.h"
#include "Engine/Framework/IUISubsystem.h"
#include "Engine/Framework/ISubsystem.h"
#include "Engine/Framework/IModule.h"

#include "Engine/Framework/Impl/DefaultInputManager.h"

#include "Engine/Core/Math.h"

#include "Engine/Utils/Profiling.h"
#include "Engine/Utils/Config.h"
#include "Engine/Utils/Log.h"

#include "Engine/Scene/SceneSubsystem.h"
#include "Engine/Render/Scene/RenderSceneSubsystem.h"

namespace Cyclone
{

CDefaultApplication::CDefaultApplication()
    : m_IsInit(false)
    , m_Dt(0.0)
    , m_Platform(nullptr)
{
}

CDefaultApplication::~CDefaultApplication()
{
    DeInitImpl();
}

C_STATUS CDefaultApplication::Init(const CDefaultApplicationParams& Desc)
{
    m_Platform = Desc.Platform;
    m_Renderer = Desc.Renderer;
    m_InputManager = Desc.InputManager;
    m_UI = Desc.UIModule;

    C_ASSERT_RETURN_VAL(m_Platform, C_STATUS::C_STATUS_INVALID_ARG);

    C_STATUS Result = C_STATUS::C_STATUS_OK;

    if (m_Renderer == nullptr)
        return C_STATUS::C_STATUS_INVALID_ARG;

    if (m_InputManager)
    {
        Result = m_InputManager->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    if (m_Renderer)
    {
        CRendererDesc RendererDesc = CRendererDesc()
            .SetApplication(this);

        RendererDesc.FramesInFlightCount = GET_CONFIG()["Render"].value("FramesInFlightCount", RendererDesc.FramesInFlightCount);

        RendererDesc.EnableMultiThreading = GET_CONFIG()["Render"].value("EnableMultiThreading", RendererDesc.EnableMultiThreading);
        GStartupArguments()->GetParameter("RenderThreadEnable", RendererDesc.EnableMultiThreading);

        Result = m_Renderer->Init(&RendererDesc);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    // Initialize windows
    uint32 WindowCount = Desc.WindowCount;
    GetStartupArguments()->GetParameter("-WindowCount", WindowCount);

    C_ASSERT_RETURN_VAL(WindowCount >= 1, C_STATUS::C_STATUS_INVALID_ARG);

    m_Windows.resize(WindowCount);
    for (uint32 i = 0; i < m_Windows.size(); ++i)
    {
        auto& Window = m_Windows[i];

        Window = m_Platform->CreateWindowPtr();

        if (Window == nullptr)
            return C_STATUS::C_STATUS_INVALID_ARG;

        WindowDesc WindowParams{};
        WindowParams.App = this;
        WindowParams.PlatformDataPtr = Desc.PlatformStartupDataRawPtr;
        WindowParams.Name = "Window" + ToString(i);
        WindowParams.Title = Desc.WindowCaption;
        if (i != 0)
            WindowParams.Title += " " + ToString(i);

        GetStartupArguments()->GetParameter("-WindowSize", WindowParams.Size);

        Result = Window->Init(&WindowParams);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

        if (m_OnWindowAddedDelegate)
            m_OnWindowAddedDelegate(Window.get());
    }

    Result = OnAddSubsystems();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    if (m_UI)
    {
        Result = m_UI->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    Result = OnInit();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    m_IsInit = true;

    return C_STATUS::C_STATUS_OK;
}

void CDefaultApplication::DeInit()
{
    DeInitImpl();
}

void CDefaultApplication::DeInitImpl()
{
    if (m_IsInit == false)
        return;

    WaitAllPendingJobs();

    // Delete all Subsystems
    while(m_Subsystems.empty() == false)
    {
        RemoveSubsystem(m_Subsystems.back().get());
    }

    if (m_UI)
    {
        m_UI->Shutdown();
        m_UI = nullptr;
    }

    for (uint32 i = 0; i < m_Windows.size(); ++i)
    {
        if (auto& Window = m_Windows[i])
        {
            if (m_OnWindowRemovedDelegate)
                m_OnWindowRemovedDelegate(Window.get());

            if (Window)
            {
                Window->DeInit();
            }

            Window.reset();
        }
    }
    m_Windows.clear();

    if (m_Renderer)
    {
        m_Renderer->DeInit();
        m_Renderer.reset();
    }

    m_Platform = nullptr;
    m_IsInit = false;

    GEngineUnRegisterAllModules();
}


int CDefaultApplication::Run()
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
        OPTICK_FRAME("MainThread");

        Timestamp.Stop();
        m_Dt = Timestamp.Seconds();
        Timestamp.Start();

        NeedToExit = Frame() != C_STATUS::C_STATUS_OK;
    }

    return 0;
}

IInputHandler* CDefaultApplication::GetInputHandler()
{
    return m_InputManager.get();
}

IInputManager* CDefaultApplication::GetInputManager()
{
    return m_InputManager.get();
}

double CDefaultApplication::GetDeltaTime() const
{
    return m_Dt;
}

void CDefaultApplication::WaitAllPendingJobs()
{
    if (m_Renderer)
        m_Renderer->WaitGPU();
}

C_STATUS CDefaultApplication::Frame()
{
    C_STATUS Result = BeginFrame();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    if (Result == C_STATUS::C_STATUS_SHOULD_EXIT)
        return Result;

    Result = Update();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = EndFrame();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CDefaultApplication::BeginFrame()
{
    LOG_TRACE("Main: BeginFrame");

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

    // #todo_vk propagate changes to render objects to start render frame
    // here

    C_STATUS Result = OnBeginFrame();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

     if (m_Renderer)
     {
         C_STATUS Result = m_Renderer->EnqueueFrame();
         C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
     }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CDefaultApplication::Update()
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

    if (m_Renderer)
    {
        // #todo_vk_ui_first #todo_7 fixme crude hack, ImGUI needs to be parallelized
        m_Renderer->EnqueueFrameUI();
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

C_STATUS CDefaultApplication::EndFrame()
{
    // Wait RenderThread's Frame
    if (m_Renderer)
    {
        C_STATUS Result = m_Renderer->WaitFrame();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    if (m_UI)
    {
        C_STATUS Result = m_UI->OnEndFrame();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    // OnEndFrame Callback
    {
        C_STATUS Result = OnEndFrame();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CDefaultApplication::OnAddSubsystems()
{
    // Create default subsystems

    // #todo_subsystem add new ones
    {
        CSceneSubsystem* SceneSys = new CSceneSubsystem();
        C_STATUS Result = SceneSys->Init(this);
        CASSERT(C_SUCCEEDED(Result));

        Result = AddSubsystem(SceneSys);
        CASSERT(C_SUCCEEDED(Result));
    }
    {
        Render::CRenderSceneSubsystem* RendSceneSys = new Render::CRenderSceneSubsystem();
        C_STATUS Result = RendSceneSys->Init(this);
        CASSERT(C_SUCCEEDED(Result));

        Result = AddSubsystem(RendSceneSys);
        CASSERT(C_SUCCEEDED(Result));
    }
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CDefaultApplication::AddSubsystem(ISubsystem* Subsystem)
{
    m_Subsystems.emplace_back(Subsystem);

    C_STATUS Result = Subsystem->OnRegister();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    if (m_OnSubsystemRegisteredDelegate)
        m_OnSubsystemRegisteredDelegate(Subsystem);

    return Result;
}

void CDefaultApplication::RemoveSubsystem(ISubsystem* Subsystem)
{
    CASSERT(Subsystem);

    for (auto it = m_Subsystems.begin(); it != m_Subsystems.end(); ++it)
    {
        if (it->get() == Subsystem)
        {
            if (m_OnSubsystemUnRegisteredDelegate)
                m_OnSubsystemUnRegisteredDelegate(Subsystem);

            Subsystem->OnUnRegister();

            m_Subsystems.erase(it);

            return;
        }
    }
}

ISubsystem* CDefaultApplication::GetSubsystem(std::string_view Name)
{
    for (auto it = m_Subsystems.begin(); it != m_Subsystems.end(); ++it)
    {
        if ((*it)->GetName() == Name)
        {
            return it->get();
        }
    }
    return nullptr;
}

C_STATUS CDefaultApplication::OnInit()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CDefaultApplication::OnBeginFrame()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CDefaultApplication::OnUpdate()
{
    // do nothing
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CDefaultApplication::OnUpdateUI()
{
    // do nothing
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CDefaultApplication::OnEndFrame()
{
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
