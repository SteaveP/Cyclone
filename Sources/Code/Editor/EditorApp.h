#pragma once

#include "Engine/Framework/Impl/DefaultApplication.h"

#include "Engine/Scene/Camera.h"
#include "Engine/Core/Math.h"

namespace Cyclone
{

class CScene;
class CSceneViewport;

class EditorApplication : public DefaultApplication
{
protected:
	C_STATUS OnUpdate() override;
	C_STATUS OnRender() override;
	C_STATUS OnUpdateUI() override;
	C_STATUS OnInit() override;

protected:
	void ShowMenu();
	void ShowViewports();
	void ShowWorldOutliner();
	void ShowProperties();
	void ShowContentBrowser();

protected:
    Vector<Ptr<CScene>> m_Scenes;
    Vector<Ptr<CSceneViewport>> m_Viewports;
};

} // namespace Cyclone
