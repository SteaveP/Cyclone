#pragma once

#include "Engine/Framework/Impl/DefaultApplication.h"

namespace Cyclone
{

class EditorApplication : public DefaultApplication
{
protected:
	C_STATUS OnUpdate() override;
	C_STATUS OnRender() override;
	C_STATUS OnUpdateUI() override;
	C_STATUS OnInit() override;

protected:
	void ShowMenu();
	void ShowViewport();
	void ShowWorldOutliner();
	void ShowProperties();
	void ShowContentBrowser();

};

} // namespace Cyclone
