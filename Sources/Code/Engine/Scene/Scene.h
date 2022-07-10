#pragma once

#include "Engine/Core/Math.h"
#include "Engine/EngineModule.h"

namespace Cyclone
{

class ENGINE_API CScene
{
public:
    DISABLE_COPY_ENABLE_MOVE(CScene);

    CScene();
    ~CScene();

    const String& GetName() const { return m_Name; }

protected:
    String m_Name;

    friend class CSceneSubsystem;
};

}
