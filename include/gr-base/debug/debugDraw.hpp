#pragma once

#include "gr-base/debug/debugLine.hpp"

#include "gr-base/prefab/default.hpp"

#include <gr-math/gmath.hpp>
#include <gr-math/vector3.hpp>

#include <gr-render/gVertexArray.h>

#include <memory>
#include <vector>

#include <gr-render/vertex_array.hpp>


class DebugDraw
{
public:
    static void Initialize();

    static void Shutdown();

    static void DrawLine(const Vector3 &start, const Vector3 &end, const Color &color = COLOR_GREEN);

    static void Render();

private:
    static DebugDraw *s_DebugDraw;
    
    DebugLine debugLine;

    DebugDraw();
    ~DebugDraw();

    void render();
};



