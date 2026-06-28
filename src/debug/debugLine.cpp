#include "gr-base/debug/debugLine.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/prefab/shaderprefab.hpp"
#include <gr-render/gVertexArray.h>

DebugLine::DebugLine() : m_shader(nullptr), m_vao(nullptr), m_vbo(nullptr)
{
    gr::buffer_layout layout = {
        {gr::shader_data_type::Float4, "in_pos"},
        {gr::shader_data_type::Float4, "in_color"}
    };

    m_vbo = gr::vertex_buffer::create(nullptr, 256 * sizeof(DebugVertex), gr::buffer_usage::dynamic_draw);
    m_vbo->SetLayout(layout);

    m_vao = gr::vertex_array::create();
    m_vao->Bind();
    m_vao->AddVertexBuffer(m_vbo);
    m_vao->Unbind();
}

DebugLine::~DebugLine()
{
    if (m_vao != nullptr)
        delete m_vao;

    if (m_vbo != nullptr)
        delete m_vbo;
}

bool DebugLine::initialize()
{
    if (m_shader != nullptr)
        return true;

    ShaderPrefab* prefab = nullptr;
    try
    {
        prefab = AssetsManager::LoadShader("shader/line.glsl");
    } catch (const std::exception& e)
    {
        Console::LogError("[DebugDraw] Fail in to load shader to line '%s'!!!", e.what());
        return false;
    }

    m_shader = prefab->Get(ShaderPrefabFlags_none);
    return true;
}

void DebugLine::drawLine(const Vector3 &start, const Vector3 &end, const Color &color)
{
    m_lines.push_back({start, color});
    m_lines.push_back({end, color});
}

void DebugLine::render()
{
    m_shader->bind();

    m_vao->Bind();

    m_vbo->Bind();
    m_vbo->SetData(m_lines.data(), sizeof(DebugVertex) * m_lines.size());

    gr::gVertexArray::DrawArrays(PrimitiveType::LINES, m_lines.size());

    m_shader->unbind();
    m_vao->Unbind();
    m_vbo->Unbind();

    m_lines.clear();
}

