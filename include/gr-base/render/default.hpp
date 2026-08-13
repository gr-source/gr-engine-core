#pragma once

#include <cstdint>

class RenderTarget;

struct RenderTargetID
{
    uint32_t value;
};

static constexpr RenderTargetID InvalidRenderTarget = { UINT32_MAX };

struct RenderPipelineID
{
    uint32_t value;
};

static constexpr RenderPipelineID RenderPipelineForward = { 0 };


