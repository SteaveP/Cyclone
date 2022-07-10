#include "Resource.h"

namespace Cyclone::Render
{

CResource::CResource() = default;
CResource::~CResource()
{
    DeInitImpl();
}

C_STATUS CResource::Init(const CResourceDesc& Desc)
{
    m_Desc = Desc;
    m_TraceableLayout = m_Desc.InitialLayout;
    m_TraceableTextureUsageType = m_Desc.Texture.InitialUsage;

    return C_STATUS::C_STATUS_OK;
}

void CResource::DeInit()
{
    DeInitImpl();
}

void CResource::DeInitImpl() noexcept
{
    // nothing to do
}

} // namespace Cyclone::Render
