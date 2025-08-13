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
    static void DrawLine(const Vector3 &start, const Vector3 &end);

    static void Render(const Camera *camera);

private:
    static DebugDraw &GetInstance();

    struct Line3D
    {
        Vector3 position;

        Color color;
    };

    class LineRender
    {
    public:
        std::shared_ptr<grr::gVertexArray> m_vertexarray;

        grm::uint32 m_vbo;

        grr::gShader *m_shader;

        LineRender();

        void render(const Camera *camera, const Vector3 *data, std::size_t size);
    };
    
    std::vector<Vector3> m_lines;

    LineRender m_linerender;

    DebugDraw();
    ~DebugDraw();

    void drawLine(const Vector3 &start, const Vector3 &end);

    void render(const Camera *camera);
};



