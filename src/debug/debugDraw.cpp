#include "gr-base/debug/debugDraw.hpp"

#include "gr-base/core/logger.hpp"

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

void DebugDraw::DrawLine(const Vector3 &start, const Vector3 &end, const Color &color)
{
    s_DebugDraw->debugLine.drawLine(start, end, color);
}

void DebugDraw::Render()
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
    if (!debugLine.initialize())
        return;


    debugLine.render();
}




