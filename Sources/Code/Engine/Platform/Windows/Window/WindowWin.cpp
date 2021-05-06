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
        static HINSTANCE user32_dll = ::LoadLibraryA("user32.dll"); // Reference counted per-process
        if (PFN_SetThreadDpiAwarenessContext SetThreadDpiAwarenessContextFn = (PFN_SetThreadDpiAwarenessContext)::GetProcAddress(user32_dll, "SetThreadDpiAwarenessContext"))
        {
            SetThreadDpiAwarenessContextFn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            return;
        }
    }
    //if (IsWindows8Point1OrGreater())
    {
        static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll"); // Reference counted per-process
        if (PFN_SetProcessDpiAwareness SetProcessDpiAwarenessFn = (PFN_SetProcessDpiAwareness)::GetProcAddress(shcore_dll, "SetProcessDpiAwareness"))
        {
            SetProcessDpiAwarenessFn(PROCESS_PER_MONITOR_DPI_AWARE);
            return;
        }
    }

    SetProcessDPIAware();
}

LRESULT CALLBACK CycloneWndProc(HWND hWnd, uint32 message, WPARAM wParam, LPARAM lParam)
{
    Cyclone::WindowWinApi* window = reinterpret_cast<Cyclone::WindowWinApi*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    // #todo_window double clicking (don't forget about window style), minimizing/maximizing, esc-handling and key-modifiers handle
    // also need to properly handle ui and don't send action to core system if ui is handle key/mouse event

    if (window && window->GetApp())
    {
        IPlatform* Platform = window->GetApp()->GetPlatform();
        if (Platform)
        {
            WindowMessageParamWin Params = {};
            Params.hWnd = hWnd;
            Params.message = message;
            Params.wParam = wParam;
            Params.lParam = lParam;

            if (Platform->OnWindowMessage(window, &Params) == C_STATUS::C_STATUS_OK)
                return 0;
        }
    }

    switch (message)
    {
    case WM_CREATE:
        {
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;

    case WM_ACTIVATE:
        if (window)
        {
            bool active = wParam != WA_INACTIVE;
            window->SetActive(active);
        }
        return 0;

    case WM_KEYDOWN:
        if (window && window->GetApp() && window->GetApp()->GetInputHandler())
        {
            bool prevIsDown = (lParam & (1 << 30));
            window->GetApp()->GetInputHandler()->OnKeyDown(static_cast<int>(wParam), prevIsDown);
        }
        return 0;

    case WM_KEYUP:
        if (window && window->GetApp() && window->GetApp()->GetInputHandler())
        {
            window->GetApp()->GetInputHandler()->OnKeyUp(static_cast<int>(wParam));
        }
        return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        if (window && window->GetApp() && window->GetApp()->GetInputHandler())
        {
            // #todo_input check additional keys like Ctrl, Shift, Alt (GetKeyState(VK_MENU) < 0)

            Cyclone::MouseKey key = Cyclone::MouseKeyLeft;

            if (message == WM_RBUTTONDOWN || message == WM_RBUTTONUP)
                key = Cyclone::MouseKeyRight;
            else if (message == WM_MBUTTONDOWN || message == WM_MBUTTONUP)
                key = Cyclone::MouseKeyMiddle;

            bool isDown = message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN;

            if (isDown)
                window->GetApp()->GetInputHandler()->OnMouseDown(key);
            else
                window->GetApp()->GetInputHandler()->OnMouseUp(key);
        }
        return 0;

    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
        if (window && window->GetApp() && window->GetApp()->GetInputHandler())
        {
            Cyclone::MouseKey key = GET_XBUTTON_WPARAM(wParam) == 1 ? Cyclone::MouseKeyX1 : Cyclone::MouseKeyX2;

            bool isDown = message == WM_XBUTTONDOWN;
            if (isDown)
                window->GetApp()->GetInputHandler()->OnMouseDown(key);
            else
                window->GetApp()->GetInputHandler()->OnMouseUp(key);

        }
        return TRUE; // docs says that need to return TRUE

#if 0
    case WM_MOUSEMOVE:
        if (window && window->GetApp() && window->GetApp()->GetInputHandler())
        {
            POINTS point = MAKEPOINTS(lParam); // return point related to client area, can be negative
            window->GetApp()->GetInputHandler()->OnMouseMove(point.x, point.y);
        }
        return 0;
#endif

    case WM_MOUSEHWHEEL:
    case WM_MOUSEWHEEL:
        if (window && window->GetApp() && window->GetApp()->GetInputHandler())
        {
            short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            float relativeDelta = (float)zDelta / (float)WHEEL_DELTA;

            bool isVertical = message == WM_MOUSEWHEEL;

            if (isVertical)
                window->GetApp()->GetInputHandler()->OnMouseWheel(relativeDelta);
            else
                window->GetApp()->GetInputHandler()->OnMouseHWheel(relativeDelta);

        }
        return 0;

    case WM_DPICHANGED:
        {
            // propagate DPI to window
            if (window)
            {
                float newDPI = LOWORD(wParam);
                window->OnDPIChanged(newDPI, window->GetDPI());

            }
            // resize window to suggested size with respect do DPI
            if (window && window->ScaleWindowWithRespectToDPI())
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
        if (window)
        {
            uint32 newWidth = LOWORD(lParam);
            uint32 newHeight = HIWORD(lParam);
            window->OnResize(newWidth, newHeight);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

} // namespace anonymous

WindowWinApi::WindowWinApi()
    : m_hWnd(0)
    , m_hInstance(0)
    , m_width(0)
    , m_height(0)
    , m_dpi(C_DEFAULT_DPI)
    , m_windowActive(false)
    , m_centerCursor(false)
    , m_showCursor(true)
    , m_pendingCursorVisibility(m_showCursor)
    , m_scaleWindowWithRespectToDPI(false)
    , m_app(nullptr)
{

}

WindowWinApi::~WindowWinApi()
{
    Deinit();
}

C_STATUS WindowWinApi::Init(const WindowParams* generic_params)
{
    const WinApiWindowParams* params = reinterpret_cast<const WinApiWindowParams*>(generic_params);

    m_app = params->app;
    m_width = params->width;
    m_height = params->height;
    int cmdShowFlag = SW_SHOWDEFAULT;

    // #todo_fixme
    if (m_hInstance == NULL)
    {
        m_hInstance = GetModuleHandle(NULL);
    }

    m_windowActive = false;

    SetDPIAwareness();

    MultiByteToWideChar(CP_UTF8, 0, params->title.c_str(), (int)params->title.size(), szTitle, MAX_LOADSTRING);
    wsprintf(szWindowClass, L"SampleAppWindowClass");

    CycloneRegisterWindowClass(m_hInstance);

    if (!CycloneCreateWindow(m_hInstance, m_width, m_height, cmdShowFlag, &m_hWnd))
    {
        return C_STATUS::C_STATUS_ERROR;
    }

    // update initial state like mouse position callbacks
    OnUpdate();
    OnUpdateAfter();

    return C_STATUS::C_STATUS_OK;
}

BOOL WindowWinApi::CycloneCreateWindow(HINSTANCE hInstance, int windowWidth, int windowHeight, int nCmdShow, HWND* out_hwnd)
{
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW;

    RECT windowRect{ 0, 0, windowWidth, windowHeight };
    {
        // place window at screen's center (without DPI scaling, it would be done later)

        // #todo here don't get into account system windows scaling
        windowRect.left = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
        windowRect.top = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;
        windowRect.right = windowRect.left + windowWidth;
        windowRect.bottom = windowRect.top + windowHeight;

        AdjustWindowRect(&windowRect, dwStyle, FALSE);
    }

    HWND hWnd = CreateWindow(szWindowClass, szTitle, dwStyle,
        windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, hInstance, this);

    if (!hWnd)
    {
        return FALSE;
    }

    // retrieve DPI from window (no wat to properly to know it before window creation)
    uint32 dpi = GetDpiForWindow(hWnd);
    m_dpi = (float)dpi;

    // rescale window with respect to DPI and place to center of window
    if (m_scaleWindowWithRespectToDPI)
    {
        // place window at screen's center with respect to DPI
        windowWidth = MulDiv(windowWidth, dpi, C_DEFAULT_DPI);
        windowHeight = MulDiv(windowHeight, dpi, C_DEFAULT_DPI);

        // #todo here don't get into account system windows scaling
        windowRect.left = (GetSystemMetricsForDpi(SM_CXSCREEN, dpi) - windowWidth) / 2;
        windowRect.top = (GetSystemMetricsForDpi(SM_CYSCREEN, dpi) - windowHeight) / 2;
        windowRect.right = windowRect.left + windowWidth;
        windowRect.bottom = windowRect.top + windowHeight;

        AdjustWindowRectExForDpi(&windowRect, dwStyle, FALSE, dwExStyle, dpi);
        SetWindowPos(hWnd, nullptr, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }


    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    *out_hwnd = hWnd;

    return TRUE;
}

void WindowWinApi::Deinit()
{

}

void WindowWinApi::OnUpdate()
{
    if (IsActive() && GetApp() && GetApp()->GetInputHandler())
    {
        POINT point = {};
        BOOL res = GetCursorPos(&point);
        CASSERT(res);

        // #todo_mouse need this?
        res = ScreenToClient(m_hWnd, &point);
        CASSERT(res);

        GetApp()->GetInputHandler()->OnMouseMove((short)point.x, (short)point.y);
    }

    if (IsActive() && GetCenterCursor())
    {
        // reset mouse pos to window's center

        POINT point = {};
        point.x = m_width / 2;
        point.y = m_height / 2;

        BOOL result = ClientToScreen(m_hWnd, &point);
        CASSERT(result);

        result = SetCursorPos(point.x, point.y);
        CASSERT(result);
    }

    if (IsActive() && m_pendingCursorVisibility != m_showCursor)
    {
        // ShowCursor use internal counter to show/hide cursor
        m_showCursor = m_pendingCursorVisibility;
        int counter = ShowCursor(GetShowCursor());
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
            GetApp()->GetInputHandler()->OnMouseMove(short(m_width / 2), short(m_height / 2));
        }
    }
}

void WindowWinApi::OnResize(unsigned int newWidth, unsigned int newHeight)
{
    m_width = newWidth;
    m_height = newHeight;

    if (m_app && m_app->GetRenderer())
    {
        m_app->GetRenderer()->OnResize(this);
    }
}

void WindowWinApi::OnDPIChanged(float newDPI, float oldDPI)
{
    m_dpi = newDPI;

    if (GetApp() && GetApp()->GetPlatform())
    {
        GetApp()->GetPlatform()->OnDPIChanged(newDPI, oldDPI);
    }    
}

C_STATUS WindowWinApi::UpdateMessageQueue()
{
    MSG msg = {};
    
    // read all messages
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) && msg.message != WM_QUIT)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    bool IsExitRequested = msg.message == WM_QUIT;
    return IsExitRequested ? C_STATUS::C_STATUS_SHOULD_EXIT : C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
