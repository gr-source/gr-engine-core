#pragma once

#include "gr-base/prefab/default.hpp"

#include "gr-base/script/camera.hpp"
#include "gr-base/script/default.hpp"
#include <gr-math/color.hpp>
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

    static void DrawLine(const Vector3 &start, const Vector3 &end, const Color &startColor, const Color &endColor);

    static void DrawLine(const Vector3 &start, const Vector3 &end, const Color& color);

    static void DrawLine(const Vector3 &start, const Vector3 &end);

    static void Render();

    static void Clear();

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
        std::shared_ptr<gr::vertex_array> m_vertex_array;

        std::shared_ptr<gr::vertex_buffer> m_buffer;

        ShaderPrefab* m_shaderPrefab;

        gr::Shader* m_shader;

        LineRender();

        void init();

        void render(const Line3D *data, std::size_t size);
    };
    
    std::vector<Line3D> m_lines;

    LineRender m_linerender;

    DebugDraw();
    ~DebugDraw();

    void drawLine(const Vector3 &start, const Vector3 &end, const Color &startColor, const Color &endColor);

    void render();

    void clear();
};



