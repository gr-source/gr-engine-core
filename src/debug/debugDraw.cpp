#include "gr-base/debug/debugDraw.hpp"

#include "gr-base/core/logger.hpp"
#include <gr-math/gmath.hpp>

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

DebugDraw* DebugDraw::s_DebugDraw = nullptr;

// public
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

void DebugDraw::DrawPoint(const Vector3& position, const Color& color)
{
    s_DebugDraw->debugBuffer.drawPoint(position, color);
}

void DebugDraw::DrawLine(const Vector3 &start, const Vector3 &end, const Color &color)
{
    s_DebugDraw->debugBuffer.drawLine(start, end, color);
}

void DebugDraw::DrawSphere(const Vector3& center, float radius, const Color &color)
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

            s_DebugDraw->debugBuffer.drawLine(lastPoint, temp, color);
            s_DebugDraw->debugBuffer.drawLine(lastPoint, cache[n], color);

            cache[n] = lastPoint;
            lastPoint = temp;
        }
    }
}

void DebugDraw::Renderer()
{
    s_DebugDraw->render();
}

// private
DebugDraw::DebugDraw()
{
    logger::info("[ DebugDraw ] Initialize '%p'.", this);
}

DebugDraw::~DebugDraw()
{
    logger::info("[ DebugDraw ] Shutdown '%p'.", this);
}

void DebugDraw::render()
{
    // if (!debugBuffer.initialize())
        // return;

    debugBuffer.renderer();
}




