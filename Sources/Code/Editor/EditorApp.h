#pragma once

#include "Engine/Framework/Impl/DefaultApplication.h"


#include "Engine/Scene/Camera.h"
#include "Engine/Core/Math.h"

namespace Cyclone
{

struct CViewport
{
	Ptr<IWindow> Window;
	// viewport in window in pixels
    Vec2 UpperLeftCorner;
    Vec2 BottomRightCorner;
	
	CCameraPtr Camera;
};

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
	Vector<CViewport> m_Viewports;
};

} // namespace Cyclone
