#pragma once

#include "Engine/Framework/IWindow.h"

#include "../Common/CommonWin.h"

namespace Cyclone
{

class IApplication;

struct WindowMessageParamWin
{
    HWND hWnd;
    UINT Message;
    WPARAM wParam;
    LPARAM lParam;
};

struct WinApiWindowParams : public WindowParams
{
    HINSTANCE hInstance;
    int nCmdShow;
};

class WindowWinApi : public IWindow
{
public:
    WindowWinApi();
    virtual ~WindowWinApi();

    virtual C_STATUS Init(const WindowParams* Params) override;
    virtual void Deinit() override;

    virtual C_STATUS UpdateMessageQueue() override;

    virtual void OnUpdate() override;
    virtual void OnUpdateAfter() override;
    virtual void OnResize(unsigned int NewWidth, unsigned int NewHeight) override;
    virtual void OnDPIChanged(float NewDPI, float OldDPI) override;

    virtual int GetWidth() const override { return m_Width; }
    virtual int GetHeight() const override { return m_Height; }

    virtual PlatformWindowHandle GetPlatformWindowHandle() const override { return reinterpret_cast<PlatformWindowHandle>(m_hWnd); };
    virtual IApplication* GetApp() const override { return m_App; }

    virtual void SetActive(bool Active) override { m_WindowActive = Active; }
    virtual bool IsActive() const override { return m_WindowActive; }

    virtual void SetShowCursor(bool Show) override { m_PendingCursorVisibility = Show; }
    virtual bool GetShowCursor() const override { return m_ShowCursor; }

    virtual void SetCenterCursor(bool CenterCursor) override { m_CenterCursor = CenterCursor; }
    virtual bool GetCenterCursor() const override{ return m_CenterCursor; }

    virtual float GetDPI() const { return m_Dpi; }

    bool ScaleWindowWithRespectToDPI() const { return m_ScaleWindowWithRespectToDPI; }



protected:
    BOOL CycloneCreateWindow(HINSTANCE, int WindowWidth, int WindowHeight, int, HWND* Out_hwnd);

protected:
    HWND m_hWnd;
    HINSTANCE m_hInstance;

    int m_Width;
    int m_Height;

    float m_Dpi;

    bool m_WindowActive;
    bool m_CenterCursor;
    bool m_ShowCursor;
    bool m_PendingCursorVisibility;

    bool m_ScaleWindowWithRespectToDPI; // #todo_config

    IApplication* m_App;
};

} // namespace Cyclone
