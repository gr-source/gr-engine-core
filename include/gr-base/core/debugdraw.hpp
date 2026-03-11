#pragma once

#include "gr-base/script/default.hpp"
#include <gr-math/color.hpp>
#include <gr-math/gmath.hpp>
#include <gr-math/vector3.hpp>
#include <gr-render/gVertexArray.h>
#include <memory>
#include <vector>

class DebugDraw
{
public:
    static void Initialize();

    static void Shutdown();

    static void DrawLine(const Vector3 &start, const Vector3 &end, const Color &startColor, const Color &endColor);

    static void DrawLine(const Vector3 &start, const Vector3 &end, const Color& color);

    static void DrawLine(const Vector3 &start, const Vector3 &end);

    static void Render(const Camera *camera);

    static void PosRender();

private:
    static DebugDraw *s_DebugDraw;

    typedef struct Line3D
    {
        Vector3 position;

        Color color;
    } Line3D;

    class LineRender
    {
    public:
        std::shared_ptr<grr::gVertexArray> m_vertexarray;

        u32 m_vbo;

        grr::gShader *m_shader;

        LineRender();

        void render(const Camera *camera, const Line3D *data, std::size_t size);
    };
    
    std::vector<Line3D> m_lines;

    LineRender m_linerender;

    DebugDraw();
    ~DebugDraw();

    void drawLine(const Vector3 &start, const Vector3 &end, const Color &startColor, const Color &endColor);

    void render(const Camera *camera);

    void posRender();
};



