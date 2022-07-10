#include "Fence.h"

namespace Cyclone::Render
{

CFence::CFence() = default;
CFence::~CFence()
{
    // It is safe to call this virtual function here as it is base class anyway
    DeInit();
}

C_STATUS CFence::Init(const CFenceDesc& Desc)
{
    m_Desc = Desc;
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone::Render
