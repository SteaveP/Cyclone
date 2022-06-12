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

    C_STATUS Init(IApplication* App);

    // IInputHandler
    virtual void OnKeyDown(int Key, bool PrevKeyState) override;
    virtual void OnKeyUp(int Key) override;
    virtual void OnMouseDown(MouseKey Key) override;
    virtual void OnMouseUp(MouseKey Key) override;
    virtual void OnMouseMove(short x, short y) override;
    virtual void InjectMousePositionDelta(short deltaX, short deltaY) override;
    virtual void OnMouseWheel(float relativeDelta) override;
    virtual void OnMouseHWheel(float relativeDelta) override;

    // IInputManager
    virtual void OnFrame() override;

    virtual bool IsKeyDown(int Key) override;
    virtual bool IsKeyPressed(int Key) override;

    virtual bool IsMouseDown(MouseKey Key) override;

    virtual void GetMouseCoords(short& X, short& Y) override;
    virtual void GetMouseCoordsDelta(short& X, short& Y) override;
    virtual float GetMouseWheel() override;
    virtual float GetMouseHWheel() override;

protected:
    IApplication* m_App;

    struct State
    {
        bool m_MouseButtons[MouseKeyCount];
        float m_MouseWheel;
        float m_MouseHWheel;
        short m_MouseX;
        short m_MouseY;
        short m_MouseDeltaX;
        short m_MouseDeltaY;

        static const int MaxKeys = 256;
        bool m_KeysDown[MaxKeys];
        bool m_keysPressed[MaxKeys];

        void OnFrame();

        void UpdateMouse(const State& PendingState);
        void UpdateKeyboard(const State& PendingState);

        void Reset();
        void ResetMouse(bool ResetPosition);
        void ResetKeyboard();
    };

    State m_CurrentState;
    State m_PendingState;
};

} // namespace Cyclone
