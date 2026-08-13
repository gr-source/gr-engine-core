#pragma once

#include "gr-base/core/assetPtr.hpp"

#include <gr-render/default.hpp>

struct RenderGraphBuild
{
    Handle color = HANDLE_INVALID;
    Handle depth = HANDLE_INVALID;
};

class IRenderPass
{
public:
    virtual ~IRenderPass() = default;

    virtual void Setup(RenderGraphBuild& build) = 0;

    virtual void Execute() = 0;
};

