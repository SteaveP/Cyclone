#include "Pipeline.h"

namespace Cyclone::Render
{

CPipeline::CPipeline() = default;
CPipeline::~CPipeline() = default;

Cyclone::C_STATUS CPipeline::Init(const CPipelineDesc& Desc)
{
    m_Desc = Desc;
    return C_STATUS::C_STATUS_OK;
}

void CPipeline::DeInit()
{

}

} // namespace Cyclone::Render
