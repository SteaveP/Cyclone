#include "DefaultInputManager.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Engine.h"

#include "Engine/UI/ImGui/CommonImGui.h"

namespace Cyclone
{

DefaultInputManager::DefaultInputManager()
    : m_App(nullptr)
    , m_CurrentState{}
    , m_PendingState{}
{
}

DefaultInputManager::~DefaultInputManager() = default;

C_STATUS DefaultInputManager::Init(IApplication* App)
{
    m_App = App;

    return C_STATUS::C_STATUS_OK;
}

void DefaultInputManager::OnFrame()
{
    // check flags
    bool IsUpdateMouse      = true;
    bool IsUpdateKeyboard   = true;

    // #todo move implementation to UISubsystem
    if (ImGui::GetCurrentContext() != nullptr)
    {
        IsUpdateMouse    = ImGui::GetIO().WantCaptureMouse == false;
        IsUpdateKeyboard = ImGui::GetIO().WantCaptureKeyboard == false;
    }

    // update current state if needed
    if (IsUpdateMouse)
    {
        m_CurrentState.UpdateMouse(m_PendingState);
    }
    else
    {
        m_CurrentState.ResetMouse(false);
        m_PendingState.ResetMouse(false);
    }

    if (IsUpdateKeyboard)
    {
        m_CurrentState.UpdateKeyboard(m_PendingState);
    }
    else
    {
        m_CurrentState.ResetKeyboard();
        m_PendingState.ResetKeyboard();
    }

    // clear pending state
    m_PendingState.OnFrame();
}

void DefaultInputManager::OnKeyDown(int Key, bool PrevKeyState)
{
    if (Key >= State::MaxKeys)
    {
        CASSERT(false);
        return;
    }

    m_PendingState.m_KeysDown[Key] = true;

    if (PrevKeyState)
        m_PendingState.m_keysPressed[Key] = false;
    else
        m_PendingState.m_keysPressed[Key] = true;
}

void DefaultInputManager::OnKeyUp(int Key)
{
    if (Key >= State::MaxKeys)
    {
        CASSERT(false);
        return;
    }

    m_PendingState.m_KeysDown[Key] = false;
    m_PendingState.m_keysPressed[Key] = false;
}

void DefaultInputManager::OnMouseDown(MouseKey Key)
{
    // #todo_input validation
    m_PendingState.m_MouseButtons[Key] = true;
}

void DefaultInputManager::OnMouseUp(MouseKey Key)
{
    // #todo_input validation
    m_PendingState.m_MouseButtons[Key] = false;
}

void DefaultInputManager::OnMouseMove(short X, short Y)
{
    m_PendingState.m_MouseDeltaX = X - m_PendingState.m_MouseX;
    m_PendingState.m_MouseDeltaY = Y - m_PendingState.m_MouseY;

    m_PendingState.m_MouseX = X;
    m_PendingState.m_MouseY = Y;
}

void DefaultInputManager::InjectMousePositionDelta(short DeltaX, short DeltaY)
{
    m_PendingState.m_MouseDeltaX = DeltaX;
    m_PendingState.m_MouseDeltaY = DeltaY;
}

void DefaultInputManager::OnMouseWheel(float RelativeDelta)
{
    m_PendingState.m_MouseWheel = RelativeDelta;
}

void DefaultInputManager::OnMouseHWheel(float RelativeDelta)
{
    m_PendingState.m_MouseHWheel = RelativeDelta;
}

bool DefaultInputManager::IsKeyDown(int Key)
{
    return m_CurrentState.m_KeysDown[Key];
}

bool DefaultInputManager::IsKeyPressed(int Key)
{
    return m_CurrentState.m_keysPressed[Key];
}

bool DefaultInputManager::IsMouseDown(MouseKey Key)
{
    return m_CurrentState.m_MouseButtons[Key];
}

void DefaultInputManager::GetMouseCoords(short& X, short& Y)
{
    X = m_CurrentState.m_MouseX;
    Y = m_CurrentState.m_MouseY;
}

void DefaultInputManager::GetMouseCoordsDelta(short& X, short& Y)
{
    X = m_CurrentState.m_MouseDeltaX;
    Y = m_CurrentState.m_MouseDeltaY;
}

float DefaultInputManager::GetMouseWheel()
{
    return m_CurrentState.m_MouseWheel;
}

float DefaultInputManager::GetMouseHWheel()
{
    return m_CurrentState.m_MouseHWheel;
}

void DefaultInputManager::State::OnFrame()
{
    memset(m_keysPressed, 0, sizeof(m_keysPressed));
    // prev mouse pos updated in other place
    m_MouseWheel = 0.f;
    m_MouseHWheel = 0.f;
}

void DefaultInputManager::State::UpdateMouse(const State& PendingState)
{
    for (uint32 i = 0; i < MouseKeyCount; ++i)
    {
        m_MouseButtons[i] = PendingState.m_MouseButtons[i];
    }

    m_MouseWheel = PendingState.m_MouseWheel;
    m_MouseHWheel = PendingState.m_MouseHWheel;

    m_MouseDeltaX = PendingState.m_MouseDeltaX;
    m_MouseDeltaY = PendingState.m_MouseDeltaY;

    m_MouseX = PendingState.m_MouseX;
    m_MouseY = PendingState.m_MouseY;
}

void DefaultInputManager::State::UpdateKeyboard(const State& PendingState)
{
    memcpy(m_KeysDown, PendingState.m_KeysDown, sizeof(m_KeysDown));
    memcpy(m_keysPressed, PendingState.m_keysPressed, sizeof(m_keysPressed));
}

void DefaultInputManager::State::Reset()
{
    ResetMouse(true);
    ResetKeyboard();
}

void DefaultInputManager::State::ResetMouse(bool ResetPosition)
{
    for (auto& Button : m_MouseButtons)
        Button = false;

    m_MouseWheel = 0;
    m_MouseHWheel = 0;

    m_MouseDeltaX = 0;
    m_MouseDeltaY = 0;

    if (ResetPosition)
    {
        m_MouseX = 0;
        m_MouseY = 0;
    }
}

void DefaultInputManager::State::ResetKeyboard()
{
    memset(m_KeysDown, 0, sizeof(m_KeysDown));
    memset(m_keysPressed, 0, sizeof(m_keysPressed));
}

} // namespace Cyclone
