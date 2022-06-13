#pragma once

#include "Engine/Core/Types.h"

namespace Cyclone
{

class IWindow;
class IRenderer;
class IApplication;

class Scene;

struct SceneRendererDesc
{
    IWindow* window;
    IApplication* app;
    IRenderer* renderer;
};

class ISceneRenderer
{
public:
    virtual ~ISceneRenderer() = default;

    virtual C_STATUS Init(const SceneRendererDesc* desc) = 0;
    virtual void DeInit() = 0;

    virtual C_STATUS Render() = 0;
    virtual void OnResize(const IWindow* window) = 0;

    virtual void SetScene(Ptr<Scene> scene) = 0;

    //virtual void OnStaticMeshAdded(RenderScene* renderScene, RenderStaticMeshInstancePtr instance) = 0;
    //virtual void OnStaticMeshRemoved(RenderScene* renderScene, RenderStaticMeshInstancePtr instance) = 0;
    //
    //virtual IRenderer* GetRenderer() const = 0;
    //virtual RenderScene* GetRenderScene() const = 0;
    //virtual Ptr<Scene> GetScene() const = 0;
};

} // namespace Cyclone
