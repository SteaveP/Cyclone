#pragma once

#include "Engine/Core/Types.h"

namespace Cyclone
{

class IRenderer;
struct RendererDesc;

class ENGINE_API IRendererFactory
{
public:
    DISABLE_COPY_ENABLE_MOVE(IRendererFactory);

    virtual ~IRendererFactory() = 0 {};
    virtual UniquePtr<IRenderer> CreateRenderer() = 0;
    virtual UniquePtr<RendererDesc> CreateRendererParams() = 0;
};

} // namespace Cyclone
