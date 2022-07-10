#include "Pipeline.h"

namespace Cyclone::Render
{

CPipelineState::CPipelineState() = default;
CPipelineState::~CPipelineState()
{
    DeInit();
}

C_STATUS CPipelineState::Init(const CPipelineStateDesc& Desc)
{
    m_Desc = Desc;
    return C_STATUS::C_STATUS_OK;
}

void CPipelineState::DeInit()
{
    // nothing to do
}

} // namespace Cyclone::Render
