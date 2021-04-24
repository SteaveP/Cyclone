#pragma once

#include "Engine/Framework/IInputHandler.h"
#include "Engine/Core/ErrorCodes.h"

namespace Cyclone
{

class IApplication;

class DefaultInputManager : public IInputManager, public IInputHandler
{
public:
    DefaultInputManager();
    virtual ~DefaultInputManager();

    C_STATUS Init(IApplication* app);

    // IInputHandler
    virtual void OnKeyDown(int key, bool prevKeyState) override;
    virtual void OnKeyUp(int key) override;
    virtual void OnMouseDown(MouseKey key) override;
    virtual void OnMouseUp(MouseKey key) override;
    virtual void OnMouseMove(short x, short y) override;
    virtual void InjectMousePositionDelta(short deltaX, short deltaY) override;
    virtual void OnMouseWheel(float relativeDelta) override;
    virtual void OnMouseHWheel(float relativeDelta) override;

    // IInputManager
    virtual void OnFrame() override;

    virtual bool IsKeyDown(int key) override;
    virtual bool IsKeyPressed(int key) override;

    virtual bool IsMouseDown(MouseKey key) override;

    virtual void GetMouseCoords(short& x, short& y) override;
    virtual void GetMouseCoordsDelta(short& x, short& y) override;
    virtual float GetMouseWheel() override;
    virtual float GetMouseHWheel() override;

protected:
    IApplication* m_app;

    struct State
    {
        bool m_mouseButtons[MouseKeyCount];
        float m_mouseWheel;
        float m_mouseHWheel;
        short m_mouseX;
        short m_mouseY;
        short m_mouseDeltaX;
        short m_mouseDeltaY;

        static const int MaxKeys = 256;
        bool m_keysDown[MaxKeys];
        bool m_keysPressed[MaxKeys];

        void OnFrame();

        void UpdateMouse(const State& pendingState);
        void UpdateKeyboard(const State& pendingState);

        void Reset();
        void ResetMouse(bool resetPosition);
        void ResetKeyboard();
    };

    State m_currentState;
    State m_pendingState;
};

} // namespace Cyclone
