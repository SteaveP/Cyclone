#pragma once

#include "PlatformWinModule.h"
#include "Engine/Framework/IPlatform.h"

namespace Cyclone
{

class PLATFORMWIN_API PlatformWin : public IPlatform
{
public:
    virtual WindowPtr CreateWindowPtr() override;

    virtual void ChangeWorkingDirectory(std::string_view path) override;
};

} // namespace Cyclone
