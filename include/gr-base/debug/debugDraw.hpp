#pragma once

#include "gr-base/debug/debugBuffer.hpp"

#include "gr-base/prefab/default.hpp"

#include <gr-math/gmath.hpp>
#include <gr-math/vector3.hpp>

#include <memory>
#include <vector>

#include <gr-render/vertex_array.hpp>


class DebugDraw
{
public:
    static void Initialize();

    static void Shutdown();

    static void DrawPoint(const Vector3& position, const Color& color = COLOR_WHITE);

    static void DrawLine(const Vector3 &start, const Vector3 &end, const Color &color = COLOR_GREEN);

    static void DrawSphere(const Vector3& center, float radius, const Color &color = COLOR_BLUE);

    static void Renderer();

private:
    static DebugDraw *s_DebugDraw;
    
    DebugBuffer debugBuffer;

    DebugDraw();
    ~DebugDraw();

    void render();
};



