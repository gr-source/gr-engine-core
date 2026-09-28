#pragma once

#include "gr-base/render/debug/debugVertex.hpp"

#ifndef DEBUG_DRAW_VERTEX_BUFFER_SIZE
    #define DEBUG_DRAW_VERTEX_BUFFER_SIZE 4096
#endif // DEBUG_DRAW_VERTEX_BUFFER_SIZE

struct DebugRenderContext
{
    DebugLine* lines =
        nullptr;
    size_t lineCapacity =
        0;
    size_t lineCount =
        0;

    DebugPoint* points =
        nullptr;
    size_t pointCapacity =
        0;
    size_t pointCount =
        0;

    DebugVertex vertexBuffer[DEBUG_DRAW_VERTEX_BUFFER_SIZE];
    size_t vertexCount =
        0;
};

extern DebugRenderContext* gDebugRenderContext;
