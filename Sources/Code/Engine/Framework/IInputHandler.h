#pragma once

namespace Cyclone
{

enum MouseKey
{
    MouseKeyLeft    = 0,
    MouseKeyMiddle  = 1,
    MouseKeyRight   = 2,
    MouseKeyX1      = 3,
    MouseKeyX2      = 4,

    MouseKeyCount
};

// #todo_input key mappings to enum

class ENGINE_API IInputHandler
{
public:
    DISABLE_COPY_ENABLE_MOVE(IInputHandler);

    IInputHandler() = default;
    virtual ~IInputHandler() = default;

    virtual void OnKeyDown(int Key, bool PrevKeyState) = 0;
    virtual void OnKeyUp(int Key) = 0;

    virtual void OnMouseDown(MouseKey Key) = 0;
    virtual void OnMouseUp(MouseKey Key) = 0;
    virtual void OnMouseMove(short X, short Y) = 0;
    virtual void InjectMousePositionDelta(short X, short Y) = 0;

    virtual void OnMouseWheel(float RelativeDelta) = 0;
    virtual void OnMouseHWheel(float RelativeDelta) = 0;
};

class IInputManager
{
public:
    DISABLE_COPY_ENABLE_MOVE(IInputManager);

    IInputManager() = default;
    virtual ~IInputManager() = default;

    virtual void OnFrame() = 0;

    virtual bool IsKeyDown(int Key) = 0;
    virtual bool IsKeyPressed(int Key) = 0;

    virtual bool IsMouseDown(MouseKey Key) = 0;

    virtual void GetMouseCoords(short& X, short& Y) = 0;
    virtual void GetMouseCoordsDelta(short& X, short& Y) = 0;

    virtual float GetMouseWheel() = 0;
    virtual float GetMouseHWheel() = 0;
};

} // namespace Cyclone
