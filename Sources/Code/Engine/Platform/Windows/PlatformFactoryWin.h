#pragma once

#include "PlatformWinModule.h"
#include "Engine/Framework/IPlatformFactory.h"

namespace Cyclone
{

class PLATFORMWIN_API PlatformFactoryWin : public IPlatformFactory
{
public:
    virtual WindowPtr CreateWindowPtr() override;
};

PLATFORMWIN_API void GInitPlatformFactoryWin();

} // namespace Cyclone
