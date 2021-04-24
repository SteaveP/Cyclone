#pragma once

#include "PlatformWinModule.h"
#include "Engine/Framework/IPlatform.h"
#include "Engine/Framework/IModule.h"

namespace Cyclone
{

class PLATFORMWIN_API PlatformWin : public IPlatform
{
public:
    virtual WindowPtr CreateWindowPtr() override;

    virtual void ChangeWorkingDirectory(std::string_view path) override;
};

class PLATFORMWIN_API PlatformWinModule : public IModule
{
public:
    virtual C_STATUS OnRegister() override;
    virtual C_STATUS OnUnRegister() override;
};

} // namespace Cyclone
