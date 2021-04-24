#include "DefaultApplication.h"

#include "Engine/Framework/IPlatformFactory.h"
#include "Engine/Framework/IRendererFactory.h"
#include "Engine/Framework/IRenderer.h"
#include "Engine/Framework/ISceneRenderer.h"
#include "Engine/Framework/IInputHandler.h"

#include "engine/Framework/Impl/DefaultInputManager.h"

//#include "Engine/Utils/Timestamp.h"

#include <direct.h> // #todo_fixme

namespace Cyclone
{

void changeDirectory(std::string_view path) // #todo_fixme
{
    chdir(path.data());
}

DefaultApplication::DefaultApplication()
    : m_isInit(false)
    , m_dt(0.0)
{
}

DefaultApplication::~DefaultApplication()
{
    DeInit();
}

C_STATUS DefaultApplication::Init(const DefaultApplicationParams& desc)
{
    m_isInit = true;

    // assume that app is run from <RootDir>/Bin to load binary dependencies (.dlls)
    // after that, need to change directory to <RootDir> to be able to reference assets and other project's files
    changeDirectory("..");

    m_window = GEngineGetPlatformFactory()->CreateWindowPtr();

    if (/*m_renderer == nullptr || */m_window == nullptr)
        return C_STATUS::C_STATUS_INVALID_ARG;

    // #todo parse command line

    {
        m_inputManager = std::make_unique<DefaultInputManager>();
        C_STATUS result = m_inputManager->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    // initialize window
    {
        WindowParams windowParams{};
        windowParams.app = this;
        windowParams.width = 1920;
        windowParams.height = 1080;
        windowParams.title = desc.windowCaption.empty() ? "Cyclone's Sample" : desc.windowCaption;

        C_STATUS result = m_window->Init(&windowParams);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    {
        C_STATUS result = OnInit();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    return C_STATUS::C_STATUS_OK;
}

void DefaultApplication::DeInit()
{
    if (m_isInit == false)
        return;

    WaitAllPendingJobs();

    if (m_renderer)
    {
        m_renderer->SetSceneRenderer(nullptr);
        m_renderer.reset();
    }

    if (m_window)
    {
        m_window.reset();
    }

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

    HQTimeSample timestamp;
    timestamp.Start();

    int ret = 0;
    while (true)
    {
        timestamp.Stop();
        m_dt = timestamp.Seconds();
        timestamp.Start();

        bool needToExit = false;

        needToExit = Update() != C_STATUS::C_STATUS_OK;

        if (needToExit)
        {
            break;
        }
    }

    return ret;
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

C_STATUS DefaultApplication::Update()
{
    // update window's message queues
    if (m_window && m_window->UpdateMessageQueue() == false)
    {
        return C_STATUS::C_STATUS_INVALID_ARG;
    }

    if (m_window)
    {
        m_window->OnFrame();
    }

    // handle input
    if (m_inputManager)
    {
        m_inputManager->OnFrame();
    }

    if (m_renderer)
    {
        // #todo_gfx move rendering into separate thread?
        C_STATUS result = m_renderer->BeginFrame();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    // custom update callback
    {
        C_STATUS result = OnUpdate();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    // update UI
    {
        C_STATUS result = OnUpdateUI();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    if (m_renderer)
    {
        // #todo_gfx move rendering into separate thread?

        C_STATUS result = m_renderer->Render();
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);
    }

    // #todo if exit was requested need to call OnFrameAfter() methods?

    if (m_window)
    {
        m_window->OnFrameAfter();
    }

    return C_STATUS::C_STATUS_OK;
}

Cyclone::C_STATUS DefaultApplication::OnUpdate()
{
    // do nothing
    return C_STATUS::C_STATUS_OK;
}

Cyclone::C_STATUS DefaultApplication::OnUpdateUI()
{
    // do nothing
    return C_STATUS::C_STATUS_OK;
}

Cyclone::C_STATUS DefaultApplication::OnInit()
{
    // do nothing
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
