#include "gr-base/debug/debugDraw.hpp"

#include "gr-base/render/debug/debugRenderContext.hpp"
#include <cassert>

/*
const float cx = m_boundary.x + m_boundary.w * 0.5f;
const float cz = m_boundary.y + m_boundary.h * 0.5f;

DebugDraw::DrawLine(
    {cx - 1.0f, 0.0f, cz},
    {cx + 1.0f, 0.0f, cz}
);

DebugDraw::DrawLine(
    {cx, 0.0f, cz - 1.0f},
    {cx, 0.0f, cz + 1.0f}
);
*/

// DebugDraw* DebugDraw::s_DebugDraw = nullptr;

// public
/*
void DebugDraw::Initialize()
{
    if (s_DebugDraw == nullptr)
        s_DebugDraw = new DebugDraw();
}

void DebugDraw::Shutdown()
{
    if (s_DebugDraw != nullptr)
        delete s_DebugDraw;
    s_DebugDraw = nullptr;
}
*/

namespace DebugDraw
{

void DrawPoint(const Vector3& position, const Color& color)
{
    assert(gDebugRenderContext != nullptr);

    if ((gDebugRenderContext->pointCount + 1) >= gDebugRenderContext->pointCapacity)
    {
        gDebugRenderContext->pointCapacity =
            gDebugRenderContext->pointCapacity > 0 ? gDebugRenderContext->pointCapacity * 2 : 2;

        gDebugRenderContext->points =
            (DebugPoint*)realloc(gDebugRenderContext->points, gDebugRenderContext->pointCapacity * sizeof(DebugPoint));
    }

    DebugPoint& point =
        gDebugRenderContext->points[gDebugRenderContext->pointCount++];
    point.position = position;
    point.color = color;
}

void DrawLine(const Vector3 &start, const Vector3 &end, const Color &color)
{
    assert(gDebugRenderContext != nullptr);

    if ((gDebugRenderContext->lineCount + 1) >= gDebugRenderContext->lineCapacity)
    {
        gDebugRenderContext->lineCapacity =
            gDebugRenderContext->lineCapacity > 0 ? gDebugRenderContext->lineCapacity * 2 : 2;

        gDebugRenderContext->lines =
            (DebugLine*)realloc(gDebugRenderContext->lines, gDebugRenderContext->lineCapacity * sizeof(DebugLine));
    }

    DebugLine& line =
        gDebugRenderContext->lines[gDebugRenderContext->lineCount++];
    line.start = start;
    line.end = end;
    line.color = color;
}

void DrawSphere(const Vector3& center, float radius, const Color &color)
{
    constexpr size_t stepSize = 15;
    constexpr size_t segment = 360 / stepSize;

    Vector3 cache[segment];
    for (size_t n = 0; n < segment; ++n)
        cache[n] = center + Vector3{ 0.0f, 0.0f, radius };

    Vector3 lastPoint, temp;
    for (int i = stepSize; i <= 360; i += stepSize)
    {
        const float s = sinf(Math::rad((float)i));
        const float c = cosf(Math::rad((float)i));

        lastPoint.x = center.x;
        lastPoint.y = center.y + radius * s;
        lastPoint.z = center.z + radius * c;

        for (int n = 0, j = stepSize; j <= 360; j += stepSize, ++n)
        {
            temp.x = center.x + sinf(Math::rad((float)j)) * radius * s;
            temp.y = center.y + cosf(Math::rad((float)j)) * radius * s;
            temp.z = lastPoint.z;

            DrawLine(lastPoint, temp, color);
            DrawLine(lastPoint, cache[n], color);

            cache[n] = lastPoint;
            lastPoint = temp;
        }
    }
}

}



