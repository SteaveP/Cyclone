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

class IInputHandler
{
public:
    virtual ~IInputHandler() = default;

    virtual void OnKeyDown(int key, bool prevKeyState) = 0;
    virtual void OnKeyUp(int key) = 0;

    virtual void OnMouseDown(MouseKey key) = 0;
    virtual void OnMouseUp(MouseKey key) = 0;
    virtual void OnMouseMove(short x, short y) = 0;
    virtual void InjectMousePositionDelta(short x, short y) = 0;

    virtual void OnMouseWheel(float relativeDelta) = 0;
    virtual void OnMouseHWheel(float relativeDelta) = 0;
};

class IInputManager
{
public:
    virtual ~IInputManager() = default;

    virtual void OnFrame() = 0;

    virtual bool IsKeyDown(int key) = 0;
    virtual bool IsKeyPressed(int key) = 0;

    virtual bool IsMouseDown(MouseKey key) = 0;

    virtual void GetMouseCoords(short& x, short& y) = 0;
    virtual void GetMouseCoordsDelta(short& x, short& y) = 0;

    virtual float GetMouseWheel() = 0;
    virtual float GetMouseHWheel() = 0;
};

} // namespace Cyclone
