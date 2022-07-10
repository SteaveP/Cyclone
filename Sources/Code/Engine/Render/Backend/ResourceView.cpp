#include "ResourceView.h"

namespace Cyclone::Render
{

CResourceView::CResourceView() = default;
CResourceView::~CResourceView() = default;

C_STATUS CResourceView::Init(const CResourceViewDesc& Desc)
{
    m_Desc = Desc;
    
    CASSERT(m_Desc.Backend);

    return C_STATUS::C_STATUS_OK;
}

void CResourceView::DeInit()
{
    // nothing to do
}

} // namespace Cyclone::Render
