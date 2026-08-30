#pragma once

#include "gr-base/externals/ID.hpp"

#include <cstdint>

struct RenderPipepileTag
{};

typedef ID<RenderPipepileTag, uint32_t> PipelineID;

static constexpr PipelineID PipelineForward = { 0 };

