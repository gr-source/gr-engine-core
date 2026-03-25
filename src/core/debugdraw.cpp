#include "gr-base/core/debugdraw.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/core/geometry/shaderData.hpp"

#include "gr-base/script/camera.hpp"

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

void DebugDraw::DrawLine(const Vector3 &start, const Vector3 &end, const Color &startColor, const Color &endColor)
{
    return s_DebugDraw->drawLine(start, end, startColor, endColor);
}

void DebugDraw::DrawLine(const Vector3 &start, const Vector3 &end, const Color &color)
{
    return s_DebugDraw->drawLine(start, end, color, color);
}

void DebugDraw::DrawLine(const Vector3 &start, const Vector3 &end)
{
    return s_DebugDraw->drawLine(start, end, Color::white, Color::blue);
}

void DebugDraw::Render(const Camera *camera)
{
    return s_DebugDraw->render(camera);
}

void DebugDraw::Clear()
{
    return s_DebugDraw->clear();
}

// private
DebugDraw::DebugDraw()
{}

DebugDraw::~DebugDraw()
{}

void DebugDraw::drawLine(const Vector3 &start, const Vector3 &end, const Color &startColor, const Color &endColor)
{
    m_lines.push_back({start, startColor});
    m_lines.push_back({end, endColor});
}

void DebugDraw::render(const Camera *camera)
{
    if (m_linerender.m_shader != nullptr)
        m_linerender.render(camera, m_lines.data(), m_lines.size());
}

void DebugDraw::clear()
{
    m_lines.clear();
}

#include <gr-render/vertex_array.hpp>

// LineRender
DebugDraw::LineRender::LineRender() : m_shader(nullptr)
{
    ShaderParams params;
    params.base = gfile::get_data_directory("/assets/shader/line.glsl");
    params.flags = 0;
    params.type = ShaderType::LINE;
    params.name = "debug_line";

    AssetsManager::LoadShaderAsync(params, [&](gr::Shader *shader)
            {
                m_shader = shader;
            });

    gr::buffer_layout layout = {
        {gr::shader_data_type::Float3, "in_pos"},
        {gr::shader_data_type::Float4, "in_color"}
    };

    m_buffer = gr::vertex_buffer::create(nullptr, 256 * sizeof(Line3D), gr::buffer_usage::dynamic_draw);
    m_buffer->SetLayout(layout);

    m_vertex_array = gr::vertex_array::create();
    m_vertex_array->AddVertexBuffer(m_buffer);
}

void DebugDraw::LineRender::render(const Camera *camera, const Line3D *data, std::size_t size)
{
    m_buffer->Bind();
    m_buffer->SetData(data, size * sizeof(Line3D));
    m_buffer->Unbind();

    m_shader->bind();
    m_shader->set_uniform("u_projection", camera->getPerspesctive());
    m_shader->set_uniform("u_view", camera->getViewmatrix());

    m_vertex_array->Bind();

    gr::gVertexArray::DrawArrays(PrimitiveType::LINES, size);

    m_shader->unbind();
    m_vertex_array->Unbind();
}


