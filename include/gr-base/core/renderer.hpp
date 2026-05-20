#pragma once

#include "gr-base/core/default.hpp"
#include <gr-math/types.hpp>

class Renderer
{
public:
    static RenderTarget* backbuffer;

    static void Initialize();

    static void Shutdown();

    static void Process();

    static void SetRenderTarget(RenderTarget* rt);
};

