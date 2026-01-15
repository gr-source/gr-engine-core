#include "gr-base/core/debugdraw.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/core/geometry/shader.hpp"

#include "gr-base/script/camera.hpp"

// public
void DebugDraw::DrawLine(const Vector3 &start, const Vector3 &end, const Color &startColor, const Color &endColor)
{
    return GetInstance().drawLine(start, end, startColor, endColor);
}

void DebugDraw::DrawLine(const Vector3 &start, const Vector3 &end)
{
    return GetInstance().drawLine(start, end, Color::white, Color::white);
}

void DebugDraw::Render(const Camera *camera)
{
    return GetInstance().render(camera);
}

void DebugDraw::PosRender()
{
    return GetInstance().posRender();
}

// private
DebugDraw &DebugDraw::GetInstance()
{
    static DebugDraw instance;
    return instance;
}

DebugDraw::DebugDraw()
{}

DebugDraw::~DebugDraw()
{}

void DebugDraw::drawLine(const Vector3 &start, const Vector3 &end, const Color &startColor, const Color &endColor)
{
    m_lines.push_back(Line3D{start, startColor});
    m_lines.push_back(Line3D{end, endColor});
}

void DebugDraw::render(const Camera *camera)
{
    m_linerender.render(camera, m_lines.data(), m_lines.size());
}

void DebugDraw::posRender()
{
    m_lines.clear();
}

// LineRender
DebugDraw::LineRender::LineRender() : m_vbo(grr::gVertexArray::CreateBuffer(BufferType::VBO)), m_vertexarray(std::make_shared<grr::gVertexArray>()), m_shader(nullptr)
{
    ShaderParams params;
    params.base = gfile::get_data_directory("/assets/shader/line.glsl");
    params.flags = ShaderOptimize_dynamic;
    params.type = ShaderType::UNKNOWN;
    params.name = "debug_line";

    AssetsManager::LoadShaderAsync(params, [&](grr::gShader *shader){
                m_shader = shader;
            });


    m_vertexarray->bind();

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::SetAttrib(0, 3, sizeof(Line3D), reinterpret_cast<const void *>(offsetof(Line3D, position)));
    grr::gVertexArray::SetAttrib(1, 4, sizeof(Line3D), reinterpret_cast<const void *>(offsetof(Line3D, color)));
}

void DebugDraw::LineRender::render(const Camera *camera, const Line3D *data, std::size_t size)
{
    if (m_shader == nullptr || !m_shader->isValid())
        return;

    m_shader->bind();
    m_shader->set_uniform("u_projection", camera->getPerspesctive());
    m_shader->set_uniform("u_view", camera->getViewmatrix());
    // m_shader->set_uniform("u_model", Matrix4x4::identityMatrix);

    m_vertexarray->bind();

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResizeBuffer(size * sizeof(Line3D), BufferUsage::DYNAMIC);
    grr::gVertexArray::SetBufferUpdate(0, size * sizeof(Line3D), (const void *)data);

    m_shader->flush();

    grr::gVertexArray::DrawArrays(PrimitiveType::LINES, size);
}


