#pragma once

#include "Engine/Core/Types.h"

namespace Cyclone
{

class IRenderer;
struct RendererDesc;

class IRendererFactory
{
public:
    virtual ~IRendererFactory() = 0 {};
    virtual UniquePtr<IRenderer> CreateRenderer() = 0;
    virtual UniquePtr<RendererDesc> CreateRendererParams() = 0;
};

} // namespace Cyclone
