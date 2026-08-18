#pragma once

#include "gr-base/debug/debugBuffer.hpp"

class DebugDraw
{
public:
    static void Initialize();

    static void Shutdown();

    static void DrawPoint(const Vector3& position, const Color& color = Color::White());

    static void DrawLine(const Vector3 &start, const Vector3 &end, const Color &color = Color::Green());

    static void DrawSphere(const Vector3& center, float radius, const Color &color = Color::Blue());

    static void Renderer();

private:
    static DebugDraw *s_DebugDraw;
    
    DebugBuffer debugBuffer;

    DebugDraw();
    ~DebugDraw();

    void render();
};



