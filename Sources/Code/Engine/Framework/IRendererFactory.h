#pragma once

#include <memory>

namespace Cyclone
{

class IRenderer;
struct RendererDesc;

class IRendererFactory
{
public:
    virtual ~IRendererFactory() = 0 {};
    virtual std::unique_ptr<IRenderer> CreateRenderer() = 0;
    virtual std::unique_ptr<RendererDesc> CreateRendererParams() = 0;
};

} // namespace Cyclone
