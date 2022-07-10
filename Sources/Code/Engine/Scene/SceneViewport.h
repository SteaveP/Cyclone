#pragma once

#include "Engine/EngineModule.h"

#include "Engine/Framework/IWindow.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Camera.h"

namespace Cyclone
{

class ENGINE_API CSceneViewport
{
public:
    CSceneViewport();
    ~CSceneViewport();

    const String& GetName() const { return m_Name; }

public:
    Ptr<IWindow> Window;
    Ptr<CScene> Scene;

    // viewport in window in pixels
    Vec2 UpperLeftCorner;
    Vec2 BottomRightCorner;

    Ptr<CCamera> Camera;

protected:
    String m_Name;

    friend class CSceneSubsystem;
};

}
