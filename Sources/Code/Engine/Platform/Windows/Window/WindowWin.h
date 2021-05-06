#pragma once

#include "Engine/Framework/IWindow.h"

#include "../Common/CommonWin.h"

namespace Cyclone
{

class IApplication;

struct WindowMessageParamWin
{
    HWND hWnd;
    UINT message;
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

    virtual C_STATUS Init(const WindowParams* params) override;
    virtual void Deinit() override;

    virtual C_STATUS UpdateMessageQueue() override;

    virtual void OnUpdate() override;
    virtual void OnUpdateAfter() override;
    virtual void OnResize(unsigned int newWidth, unsigned int newHeight) override;
    virtual void OnDPIChanged(float newDPI, float oldDPI) override;

    virtual int GetWidth() const override { return m_width; }
    virtual int GetHeight() const override { return m_height; }

    virtual PlatformWindowHandle GetPlatformWindowHandle() const override { return reinterpret_cast<PlatformWindowHandle>(m_hWnd); };
    virtual IApplication* GetApp() const override { return m_app; }

    virtual void SetActive(bool active) override { m_windowActive = active; }
    virtual bool IsActive() const override { return m_windowActive; }

    virtual void SetShowCursor(bool show) override { m_pendingCursorVisibility = show; }
    virtual bool GetShowCursor() const override { return m_showCursor; }

    virtual void SetCenterCursor(bool centerCursor) override { m_centerCursor = centerCursor; }
    virtual bool GetCenterCursor() const override{ return m_centerCursor; }

    virtual float GetDPI() const { return m_dpi; }

    bool ScaleWindowWithRespectToDPI() const { return m_scaleWindowWithRespectToDPI; }



protected:
    BOOL CycloneCreateWindow(HINSTANCE, int windowWidth, int windowHeight, int, HWND* out_hwnd);

protected:
    HWND m_hWnd;
    HINSTANCE m_hInstance;

    int m_width;
    int m_height;

    float m_dpi;

    bool m_windowActive;
    bool m_centerCursor;
    bool m_showCursor;
    bool m_pendingCursorVisibility;

    bool m_scaleWindowWithRespectToDPI; // #todo_config

    IApplication* m_app;
};

} // namespace Cyclone
