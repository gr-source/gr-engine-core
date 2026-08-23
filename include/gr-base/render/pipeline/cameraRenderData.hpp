#pragma once

#include "gr-base/core/default.hpp"

#include <gr-render/rect.hpp>

struct CameraRenderData
{
    Rect viewport;

    TextureID color;
    TextureID depth;
};

