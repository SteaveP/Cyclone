#include "DefaultInputManager.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Engine.h"

namespace Cyclone
{

DefaultInputManager::DefaultInputManager()
    : m_app(nullptr)
    , m_currentState{}
    , m_pendingState{}
{
}

DefaultInputManager::~DefaultInputManager() = default;

C_STATUS DefaultInputManager::Init(IApplication* app)
{
    m_app = app;

    return C_STATUS::C_STATUS_OK;
}

void DefaultInputManager::OnFrame()
{
    // check flags
    // #todo_input #todo_fixme
    bool isUpdateMouse      = true; //ImGui::GetIO().WantCaptureMouse == false;
    bool isUpdateKeyboard   = true; //ImGui::GetIO().WantCaptureKeyboard == false;

    // update current state if needed
    if (isUpdateMouse)
    {
        m_currentState.UpdateMouse(m_pendingState);
    }
    else
    {
        m_currentState.ResetMouse(false);
        m_pendingState.ResetMouse(false);
    }

    if (isUpdateKeyboard)
    {
        m_currentState.UpdateKeyboard(m_pendingState);
    }
    else
    {
        m_currentState.ResetKeyboard();
        m_pendingState.ResetKeyboard();
    }

    // clear pending state
    m_pendingState.OnFrame();
}

void DefaultInputManager::OnKeyDown(int key, bool prevKeyState)
{
    if (key >= State::MaxKeys)
    {
        CASSERT(false);
        return;
    }

    m_pendingState.m_keysDown[key] = true;

    if (prevKeyState)
        m_pendingState.m_keysPressed[key] = false;
    else
        m_pendingState.m_keysPressed[key] = true;
}

void DefaultInputManager::OnKeyUp(int key)
{
    if (key >= State::MaxKeys)
    {
        CASSERT(false);
        return;
    }

    m_pendingState.m_keysDown[key] = false;
    m_pendingState.m_keysPressed[key] = false;
}

void DefaultInputManager::OnMouseDown(MouseKey key)
{
    // #todo_input validation
    m_pendingState.m_mouseButtons[key] = true;
}

void DefaultInputManager::OnMouseUp(MouseKey key)
{
    // #todo_input validation
    m_pendingState.m_mouseButtons[key] = false;
}

void DefaultInputManager::OnMouseMove(short x, short y)
{
    m_pendingState.m_mouseDeltaX = x - m_pendingState.m_mouseX;
    m_pendingState.m_mouseDeltaY = y - m_pendingState.m_mouseY;

    m_pendingState.m_mouseX = x;
    m_pendingState.m_mouseY = y;
}

void DefaultInputManager::InjectMousePositionDelta(short deltaX, short deltaY)
{
    m_pendingState.m_mouseDeltaX = deltaX;
    m_pendingState.m_mouseDeltaY = deltaY;
}

void DefaultInputManager::OnMouseWheel(float relativeDelta)
{
    m_pendingState.m_mouseWheel = relativeDelta;
}

void DefaultInputManager::OnMouseHWheel(float relativeDelta)
{
    m_pendingState.m_mouseHWheel = relativeDelta;
}

bool DefaultInputManager::IsKeyDown(int key)
{
    return m_currentState.m_keysDown[key];
}

bool DefaultInputManager::IsKeyPressed(int key)
{
    return m_currentState.m_keysPressed[key];
}

bool DefaultInputManager::IsMouseDown(MouseKey key)
{
    return m_currentState.m_mouseButtons[key];
}

void DefaultInputManager::GetMouseCoords(short& x, short& y)
{
    x = m_currentState.m_mouseX;
    y = m_currentState.m_mouseY;
}

void DefaultInputManager::GetMouseCoordsDelta(short& x, short& y)
{
    x = m_currentState.m_mouseDeltaX;
    y = m_currentState.m_mouseDeltaY;
}

float DefaultInputManager::GetMouseWheel()
{
    return m_currentState.m_mouseWheel;
}

float DefaultInputManager::GetMouseHWheel()
{
    return m_currentState.m_mouseHWheel;
}

void DefaultInputManager::State::OnFrame()
{
    memset(m_keysPressed, 0, sizeof(m_keysPressed));
    // prev mouse pos updated in other place
    m_mouseWheel = 0.f;
    m_mouseHWheel = 0.f;
}

void DefaultInputManager::State::UpdateMouse(const State& pendingState)
{
    for (uint32 i = 0; i < MouseKeyCount; ++i)
    {
        m_mouseButtons[i] = pendingState.m_mouseButtons[i];
    }

    m_mouseWheel = pendingState.m_mouseWheel;
    m_mouseHWheel = pendingState.m_mouseHWheel;

    m_mouseDeltaX = pendingState.m_mouseDeltaX;
    m_mouseDeltaY = pendingState.m_mouseDeltaY;

    m_mouseX = pendingState.m_mouseX;
    m_mouseY = pendingState.m_mouseY;
}

void DefaultInputManager::State::UpdateKeyboard(const State& pendingState)
{
    memcpy(m_keysDown, pendingState.m_keysDown, sizeof(m_keysDown));
    memcpy(m_keysPressed, pendingState.m_keysPressed, sizeof(m_keysPressed));
}

void DefaultInputManager::State::Reset()
{
    ResetMouse(true);
    ResetKeyboard();
}

void DefaultInputManager::State::ResetMouse(bool resetPosition)
{
    for (auto& button : m_mouseButtons)
        button = false;

    m_mouseWheel = 0;
    m_mouseHWheel = 0;

    m_mouseDeltaX = 0;
    m_mouseDeltaY = 0;

    if (resetPosition)
    {
        m_mouseX = 0;
        m_mouseY = 0;
    }
}

void DefaultInputManager::State::ResetKeyboard()
{
    memset(m_keysDown, 0, sizeof(m_keysDown));
    memset(m_keysPressed, 0, sizeof(m_keysPressed));
}

} // namespace Cyclone
