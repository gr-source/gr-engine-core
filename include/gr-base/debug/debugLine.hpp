#pragma once

#include <gr-render/shader.hpp>

#include <gr-render/vertex_array.hpp>

#include "gr-base/debug/debugVertex.hpp"

class DebugLine
{
public:
    DebugLine();
    ~DebugLine();

    bool initialize();

    void drawLine(const Vector3 &start, const Vector3 &end, const Color &color);

    void render();

private:
    std::vector<DebugVertex> m_lines;

    gr::vertex_array* m_vao;

    gr::vertex_buffer* m_vbo;

    gr::Shader* m_shader;
};

