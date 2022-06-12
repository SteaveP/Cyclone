#include "WindowWin.h"

#include "Engine/Engine.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IInputhandler.h"
#include "Engine/Framework/IRenderer.h"

#include "Engine/Framework/IPlatform.h"

#include <ShellScalingAPI.h>

#pragma comment(lib, "shcore")

namespace Cyclone
{

namespace // anonymous
{

#define C_DEFAULT_DPI USER_DEFAULT_SCREEN_DPI

#define MAX_LOADSTRING 100
WCHAR szWindowClass[MAX_LOADSTRING];
WCHAR szTitle[MAX_LOADSTRING];

ATOM                CycloneRegisterWindowClass(HINSTANCE hInstance);
LRESULT CALLBACK    CycloneWndProc(HWND, uint32, WPARAM, LPARAM);

typedef DPI_AWARENESS_CONTEXT(WINAPI* PFN_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT); // User32.lib + dll, Windows 10 v1607+ (Creators Update)
typedef HRESULT(__stdcall* PFN_SetProcessDpiAwareness)(_In_ PROCESS_DPI_AWARENESS value);

//////////////////////////////////////////////////////////////////////////

ATOM CycloneRegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW/* | CS_DBLCLKS*/;
    wcex.lpfnWndProc = CycloneWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(101)); // 101 is portable icon index in resource
    wcex.hIconSm = wcex.hIcon;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = szWindowClass;

    return RegisterClassExW(&wcex);
}

void SetDPIAwareness()
{
    // if (IsWindows10OrGreater()) // This needs a manifest to succeed. Instead we try to grab the function pointer!
    {
        static HINSTANCE User32_dll = ::LoadLibraryA("user32.dll"); // Reference counted per-process
        if (PFN_SetThreadDpiAwarenessContext SetThreadDpiAwarenessContextFn = (PFN_SetThreadDpiAwarenessContext)::GetProcAddress(User32_dll, "SetThreadDpiAwarenessContext"))
        {
            SetThreadDpiAwarenessContextFn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            return;
        }
    }
    //if (IsWindows8Point1OrGreater())
    {
        static HINSTANCE Shcore_dll = ::LoadLibraryA("shcore.dll"); // Reference counted per-process
        if (PFN_SetProcessDpiAwareness SetProcessDpiAwarenessFn = (PFN_SetProcessDpiAwareness)::GetProcAddress(Shcore_dll, "SetProcessDpiAwareness"))
        {
            SetProcessDpiAwarenessFn(PROCESS_PER_MONITOR_DPI_AWARE);
            return;
        }
    }

    SetProcessDPIAware();
}

LRESULT CALLBACK CycloneWndProc(HWND hWnd, uint32 Message, WPARAM wParam, LPARAM lParam)
{
    Cyclone::WindowWinApi* Window = reinterpret_cast<Cyclone::WindowWinApi*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    // #todo_window double clicking (don't forget about window style), minimizing/maximizing, esc-handling and key-modifiers handle
    // also need to properly handle ui and don't send action to core system if ui is handle key/mouse event

    if (Window && Window->GetApp())
    {
        IPlatform* Platform = Window->GetApp()->GetPlatform();
        if (Platform)
        {
            WindowMessageParamWin Params = {};
            Params.hWnd = hWnd;
            Params.Message = Message;
            Params.wParam = wParam;
            Params.lParam = lParam;

            if (Platform->OnWindowMessage(Window, &Params) == C_STATUS::C_STATUS_OK)
                return 0;
        }
    }

    switch (Message)
    {
    case WM_CREATE:
        {
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;

    case WM_ACTIVATE:
        if (Window)
        {
            bool active = wParam != WA_INACTIVE;
            Window->SetActive(active);
        }
        return 0;

    case WM_KEYDOWN:
        if (Window && Window->GetApp() && Window->GetApp()->GetInputHandler())
        {
            bool prevIsDown = (lParam & (1 << 30));
            Window->GetApp()->GetInputHandler()->OnKeyDown(static_cast<int>(wParam), prevIsDown);
        }
        return 0;

    case WM_KEYUP:
        if (Window && Window->GetApp() && Window->GetApp()->GetInputHandler())
        {
            Window->GetApp()->GetInputHandler()->OnKeyUp(static_cast<int>(wParam));
        }
        return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        if (Window && Window->GetApp() && Window->GetApp()->GetInputHandler())
        {
            // #todo_input check additional keys like Ctrl, Shift, Alt (GetKeyState(VK_MENU) < 0)

            Cyclone::MouseKey Key = Cyclone::MouseKeyLeft;

            if (Message == WM_RBUTTONDOWN || Message == WM_RBUTTONUP)
                Key = Cyclone::MouseKeyRight;
            else if (Message == WM_MBUTTONDOWN || Message == WM_MBUTTONUP)
                Key = Cyclone::MouseKeyMiddle;

            bool isDown = Message == WM_LBUTTONDOWN || Message == WM_RBUTTONDOWN || Message == WM_MBUTTONDOWN;

            if (isDown)
                Window->GetApp()->GetInputHandler()->OnMouseDown(Key);
            else
                Window->GetApp()->GetInputHandler()->OnMouseUp(Key);
        }
        return 0;

    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
        if (Window && Window->GetApp() && Window->GetApp()->GetInputHandler())
        {
            Cyclone::MouseKey Key = GET_XBUTTON_WPARAM(wParam) == 1 ? Cyclone::MouseKeyX1 : Cyclone::MouseKeyX2;

            bool isDown = Message == WM_XBUTTONDOWN;
            if (isDown)
                Window->GetApp()->GetInputHandler()->OnMouseDown(Key);
            else
                Window->GetApp()->GetInputHandler()->OnMouseUp(Key);

        }
        return TRUE; // docs says that need to return TRUE

#if 0
    case WM_MOUSEMOVE:
        if (Window && Window->GetApp() && Window->GetApp()->GetInputHandler())
        {
            POINTS Point = MAKEPOINTS(lParam); // return point related to client area, can be negative
            Window->GetApp()->GetInputHandler()->OnMouseMove(Point.x, Point.y);
        }
        return 0;
#endif

    case WM_MOUSEHWHEEL:
    case WM_MOUSEWHEEL:
        if (Window && Window->GetApp() && Window->GetApp()->GetInputHandler())
        {
            short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            float RelativeDelta = (float)zDelta / (float)WHEEL_DELTA;

            bool IsVertical = Message == WM_MOUSEWHEEL;

            if (IsVertical)
                Window->GetApp()->GetInputHandler()->OnMouseWheel(RelativeDelta);
            else
                Window->GetApp()->GetInputHandler()->OnMouseHWheel(RelativeDelta);

        }
        return 0;

    case WM_DPICHANGED:
        {
            // propagate DPI to window
            if (Window)
            {
                float NewDPI = LOWORD(wParam);
                Window->OnDPIChanged(NewDPI, Window->GetDPI());

            }
            // resize window to suggested size with respect do DPI
            if (Window && Window->ScaleWindowWithRespectToDPI())
            {
                RECT* const prcNewWindow = (RECT*)lParam;
                SetWindowPos(hWnd,
                    NULL,
                    prcNewWindow->left,
                    prcNewWindow->top,
                    prcNewWindow->right - prcNewWindow->left,
                    prcNewWindow->bottom - prcNewWindow->top,
                    SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
        return 0;

    case WM_SIZE:
        if (Window)
        {
            // #todo_window disable rendering if in background
            //if (wParam != SIZE_MINIMIZED)
            {
                uint32 NewWidth = LOWORD(lParam);
                uint32 NewHeight = HIWORD(lParam);
                Window->OnResize(NewWidth, NewHeight);
            }
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, Message, wParam, lParam);
    }
    return 0;
}

} // namespace anonymous

WindowWinApi::WindowWinApi()
    : m_hWnd(0)
    , m_hInstance(0)
    , m_Width(0)
    , m_Height(0)
    , m_Dpi(C_DEFAULT_DPI)
    , m_WindowActive(false)
    , m_CenterCursor(false)
    , m_ShowCursor(true)
    , m_PendingCursorVisibility(m_ShowCursor)
    , m_ScaleWindowWithRespectToDPI(false)
    , m_App(nullptr)
{

}

WindowWinApi::~WindowWinApi()
{
    Deinit();
}

C_STATUS WindowWinApi::Init(const WindowParams* GenericParams)
{
    const WinApiWindowParams* Params = reinterpret_cast<const WinApiWindowParams*>(GenericParams);

    m_App = Params->App;
    m_Width = Params->Width;
    m_Height = Params->Height;
    int cmdShowFlag = SW_SHOWDEFAULT;

    // #todo_fixme
    if (m_hInstance == NULL)
    {
        m_hInstance = GetModuleHandle(NULL);
    }

    m_WindowActive = false;

    SetDPIAwareness();

    MultiByteToWideChar(CP_UTF8, 0, Params->Title.c_str(), (int)Params->Title.size(), szTitle, MAX_LOADSTRING);
    wsprintf(szWindowClass, L"SampleAppWindowClass");

    CycloneRegisterWindowClass(m_hInstance);

    if (!CycloneCreateWindow(m_hInstance, m_Width, m_Height, cmdShowFlag, &m_hWnd))
    {
        return C_STATUS::C_STATUS_ERROR;
    }

    // update initial state like mouse position callbacks
    OnUpdate();
    OnUpdateAfter();

    return C_STATUS::C_STATUS_OK;
}

BOOL WindowWinApi::CycloneCreateWindow(HINSTANCE hInstance, int WindowWidth, int WindowHeight, int nCmdShow, HWND* Out_hwnd)
{
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW;

    RECT WindowRect{ 0, 0, WindowWidth, WindowHeight };

    {
        // place window at screen's center (without DPI scaling, it would be done later)

        // #todo here don't get into account system windows scaling
        WindowRect.left = (GetSystemMetrics(SM_CXSCREEN) - WindowWidth) / 2;
        WindowRect.top = (GetSystemMetrics(SM_CYSCREEN) - WindowHeight) / 2;
        WindowRect.right = WindowRect.left + WindowWidth;
        WindowRect.bottom = WindowRect.top + WindowHeight;

        AdjustWindowRect(&WindowRect, dwStyle, FALSE);
    }

    HWND hWnd = CreateWindow(szWindowClass, szTitle, dwStyle,
        WindowRect.left, WindowRect.top, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, nullptr, nullptr, hInstance, this);

    if (!hWnd)
    {
        return FALSE;
    }

    // retrieve DPI from window (no wat to properly to know it before window creation)
    uint32 Dpi = GetDpiForWindow(hWnd);
    m_Dpi = (float)Dpi;

    // rescale window with respect to DPI and place to center of window
    if (false && m_ScaleWindowWithRespectToDPI)
    {
        // place window at screen's center with respect to DPI
        WindowWidth = MulDiv(WindowWidth, Dpi, C_DEFAULT_DPI);
        WindowHeight = MulDiv(WindowHeight, Dpi, C_DEFAULT_DPI);

        // #todo here don't get into account system windows scaling
        WindowRect.left = (GetSystemMetricsForDpi(SM_CXSCREEN, Dpi) - WindowWidth) / 2;
        WindowRect.top = (GetSystemMetricsForDpi(SM_CYSCREEN, Dpi) - WindowHeight) / 2;
        WindowRect.right = WindowRect.left + WindowWidth;
        WindowRect.bottom = WindowRect.top + WindowHeight;

        AdjustWindowRectExForDpi(&WindowRect, dwStyle, FALSE, dwExStyle, Dpi);
        SetWindowPos(hWnd, nullptr, WindowRect.left, WindowRect.top, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }


    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    *Out_hwnd = hWnd;

    return TRUE;
}

void WindowWinApi::Deinit()
{

}

void WindowWinApi::OnUpdate()
{
    if (IsActive() && GetApp() && GetApp()->GetInputHandler())
    {
        POINT Point = {};
        BOOL Res = GetCursorPos(&Point);
        CASSERT(Res);

        // #todo_mouse need this?
        Res = ScreenToClient(m_hWnd, &Point);
        CASSERT(Res);

        GetApp()->GetInputHandler()->OnMouseMove((short)Point.x, (short)Point.y);
    }

    if (IsActive() && GetCenterCursor())
    {
        // reset mouse pos to window's center

        POINT Point = {};
        Point.x = m_Width / 2;
        Point.y = m_Height / 2;

        BOOL Result = ClientToScreen(m_hWnd, &Point);
        CASSERT(Result);

        Result = SetCursorPos(Point.x, Point.y);
        CASSERT(Result);
    }

    if (IsActive() && m_PendingCursorVisibility != m_ShowCursor)
    {
        // ShowCursor use internal counter to show/hide cursor
        m_ShowCursor = m_PendingCursorVisibility;
        int Counter = ShowCursor(GetShowCursor());
    }
}

void WindowWinApi::OnUpdateAfter()
{
    if (IsActive() && GetCenterCursor())
    {
        // #todo_window consider scaling and properly set cursor at window's center
        if (GetApp() && GetApp()->GetInputHandler())
        {
            // inject mouse
            GetApp()->GetInputHandler()->OnMouseMove(short(m_Width / 2), short(m_Height / 2));
        }
    }
}

void WindowWinApi::OnResize(unsigned int NewWidth, unsigned int NewHeight)
{
    m_Width = NewWidth;
    m_Height = NewHeight;

    if (m_App && m_App->GetRenderer())
    {
        //m_App->GetRenderer()->OnResize(this);
    }
}

void WindowWinApi::OnDPIChanged(float NewDPI, float OldDPI)
{
    m_Dpi = NewDPI;

    if (GetApp() && GetApp()->GetPlatform())
    {
        GetApp()->GetPlatform()->OnDPIChanged(NewDPI, OldDPI);
    }    
}

C_STATUS WindowWinApi::UpdateMessageQueue()
{
    MSG Msg = {};
    
    // read all messages
    while (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE) && Msg.message != WM_QUIT)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    bool IsExitRequested = Msg.message == WM_QUIT;
    return IsExitRequested ? C_STATUS::C_STATUS_SHOULD_EXIT : C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
