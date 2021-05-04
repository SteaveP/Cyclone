#pragma once

#include "Common/CommonVulkan.h"

namespace Cyclone::Render
{

class RenderBackendVulkan;

void DrawInit(RenderBackendVulkan* Backend);
void Draw(RenderBackendVulkan* Backend);

} // namespace Cyclone::Render
