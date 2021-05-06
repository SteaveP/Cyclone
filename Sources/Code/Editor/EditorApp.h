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

};

} // namespace Cyclone
