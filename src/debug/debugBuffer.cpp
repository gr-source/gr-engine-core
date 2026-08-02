#include "gr-base/debug/debugBuffer.hpp"

#include "gr-base/asset/assetDB.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/externals/hash.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/prefab/shaderprefab.hpp"

#include <gr-render/shader.hpp>
#include <gr-render/gl.h>

DebugBuffer::DebugBuffer() :
    m_shader(nullptr),
    m_vao(nullptr),
    m_vbo(nullptr),

    lineCount(0),
    lineCapacity(0),
    lines(nullptr),

    pointCount(0),
    pointCapacity(0),
    points(nullptr),

    vertexCount(0)
{
    gr::buffer_layout layout = {
        {gr::shader_data_type::Float4, "in_pos"},
        {gr::shader_data_type::Float4, "in_color"}
    };

    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_vbo = gr::vertex_buffer::create(nullptr, sizeof(DebugVertex), gr::buffer_usage::dynamic_draw);
    m_vbo->SetLayout(layout);

    m_vao = gr::vertex_array::create();
    m_vao->Bind();
    m_vao->AddVertexBuffer(m_vbo);
    m_vao->Unbind();

    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

DebugBuffer::~DebugBuffer()
{
    if (lines != nullptr)
        free(lines);

    if (m_vao != nullptr)
        delete m_vao;

    if (m_vbo != nullptr)
        delete m_vbo;
}

bool DebugBuffer::initialize()
{
    if (m_shader != nullptr)
        return true;

    ShaderPrefab* prefab = nullptr;
    try
    {
        prefab =
            AssetsManager::LoadShader(AssetDB::FindByHash(engine::hash::fnv1_str("engine://shader/line")));
    } catch (const std::exception& e)
    {
        Console::LogError("[DebugDraw] Fail in to load shader to line '%s'!!!", e.what());
        return false;
    }

    m_shader = prefab->Get(ShaderPrefabFlags_none);
    return true;
}


void DebugBuffer::drawPoint(const Vector3& position, const Color& color)
{
    if (pointCount >= pointCapacity)
    {
        pointCapacity = pointCapacity > 0 ? pointCapacity * 2 : 2;

        points = (DebugPoint*)realloc(points, pointCapacity * sizeof(DebugPoint));
    }

    DebugPoint& point = points[pointCount++];
    point.position = position;
    point.color = color;
}

void DebugBuffer::drawLine(const Vector3 &start, const Vector3 &end, const Color &color)
{
    if ((lineCount + 1) >= lineCapacity)
    {
        lineCapacity = lineCapacity > 0 ? lineCapacity * 2 : 2;

        lines = (DebugLine*)realloc(lines, lineCapacity * sizeof(DebugLine));
    }

    DebugLine& line = lines[lineCount++];
    line.start = start;
    line.end = end;
    line.color = color;
}

void DebugBuffer::renderer()
{
    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_shader->bind();
    m_vao->Bind();
    m_vbo->Bind();

    drawLineList();

    drawPointList();

    m_shader->unbind();
    m_vao->Unbind();
    m_vbo->Unbind();

    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugBuffer::drawLineList()
{
    for (size_t i=0;i<lineCount;i++)
        addLineVertex(lines[i]);

    flushDrawLine();

    lineCount = 0;
}

void DebugBuffer::addLineVertex(const DebugLine& line)
{
    if ((vertexCount + 2) >= DEBUG_DRAW_VERTEX_BUFFER_SIZE)
        flushDrawLine();

    auto& v0 = vertexBuffer[vertexCount++];
    auto& v1 = vertexBuffer[vertexCount++];

    v0.position = line.start;
    v0.color = line.color;

    v1.position = line.end;
    v1.color = line.color;
}

void DebugBuffer::flushDrawLine()
{
    if (vertexCount == 0)
        return;

    m_vbo->SetData(vertexBuffer, vertexCount * sizeof(DebugVertex));

    glDrawArrays(GL_LINES, 0, vertexCount);

    vertexCount = 0;
}

void DebugBuffer::drawPointList()
{
    for (size_t i=0;i<pointCount;i++)
        addPointVertex(points[i]);

    flushDrawPoint();

    pointCount = 0;
}

void DebugBuffer::addPointVertex(const DebugPoint& point)
{
    if ((vertexCount + 1) >= DEBUG_DRAW_VERTEX_BUFFER_SIZE)
        flushDrawPoint();

    auto& v0 = vertexBuffer[vertexCount++];

    v0.position = point.position;
    v0.color = point.color;
}

void DebugBuffer::flushDrawPoint()
{
    if (vertexCount == 0)
        return;

    m_vbo->SetData(vertexBuffer, vertexCount * sizeof(DebugVertex));

    glPointSize(3.0f);

    glDrawArrays(GL_POINTS, 0, vertexCount);

    vertexCount = 0;
}


