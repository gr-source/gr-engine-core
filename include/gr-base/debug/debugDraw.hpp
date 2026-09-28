#pragma once

#include <gr-render/color.hpp>

#include <gr-math/gmath.hpp>

namespace DebugDraw
{
    void DrawPoint(const Vector3& position, const Color& color = Color::White());

    void DrawLine(const Vector3 &start, const Vector3 &end, const Color &color = Color::Green());

    void DrawSphere(const Vector3& center, float radius, const Color &color = Color::Blue());
};



