#include "gr-base/render/pipeline/debugPass.hpp"

#include <gr-base/debug/debugDraw.hpp>

#include <gr-render/gRender.h>
#include <gr-render/gl.h>

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/render/debug/debugRenderContext.hpp"

DebugPass::DebugPass(
        TextureRef hdr,
        TextureRef depth) :
    m_hdr(hdr),
    m_depth(depth)
{
    gr::buffer_layout layout = {
        {gr::shader_data_type::Float4, "in_pos"},
        {gr::shader_data_type::Float4, "in_color"}
    };

    m_vbo =
        gr::vertex_buffer::create(nullptr, sizeof(DebugVertex), gr::buffer_usage::dynamic_draw);
    m_vbo->SetLayout(layout);

    m_vao =
        gr::vertex_array::create();
    m_vao->Bind();
    m_vao->AddVertexBuffer(m_vbo);
    m_vao->Unbind();
}

void DebugPass::Setup(RenderGraphBuild& build)
{
    build.Write(m_hdr);
    build.Read(m_depth);

    build.AddColorAttachment(m_hdr);
    build.SetDepthAttachment(m_depth);

    auto prefab =
        AssetsManager::LoadShader("engine://shader/line");

    m_shader =
        prefab->Get(ShaderVariantFlags_None);
}

void DebugPass::Execute()
{
    assert(gDebugRenderContext != nullptr);
    // gr::gRender::SetEnable(GR_DEPTH, false);

    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    gr::gRender::SetDepthFunc(GR_DEPTH_FUNC_LEQUAL);


    m_shader->bind();
    m_vao->Bind();
    m_vbo->Bind();

    drawLineList();

    drawPointList();

    m_shader->unbind();
    m_vao->Unbind();
    m_vbo->Unbind();
}

void DebugPass::drawLineList()
{
    for (size_t i=0;i<gDebugRenderContext->lineCount;i++)
        addLineVertex(gDebugRenderContext->lines + i);

    flushDrawLine();
}

void DebugPass::addLineVertex(const DebugLine* line)
{
    if ((gDebugRenderContext->vertexCount + 2) >= DEBUG_DRAW_VERTEX_BUFFER_SIZE)
        flushDrawLine();

    auto& v0 = gDebugRenderContext->vertexBuffer[gDebugRenderContext->vertexCount++];
    auto& v1 = gDebugRenderContext->vertexBuffer[gDebugRenderContext->vertexCount++];

    *(Vector3*)(&v0.position[0]) = line->start;
    v0.color = line->color;

    *(Vector3*)(&v1.position[0]) = line->end;
    v1.color = line->color;
}

void DebugPass::flushDrawLine()
{
    if (gDebugRenderContext->vertexCount == 0)
        return;

    m_vbo->SetData(gDebugRenderContext->vertexBuffer, gDebugRenderContext->vertexCount * sizeof(DebugVertex));

    glDrawArrays(GL_LINES, 0, gDebugRenderContext->vertexCount);

    gDebugRenderContext->vertexCount = 0;
}

void DebugPass::drawPointList()
{
    for (size_t i=0;i<gDebugRenderContext->pointCount;i++)
        addPointVertex(gDebugRenderContext->points + i);

    flushDrawPoint();
}

void DebugPass::addPointVertex(const DebugPoint* point)
{
    if ((gDebugRenderContext->vertexCount + 1) >= DEBUG_DRAW_VERTEX_BUFFER_SIZE)
        flushDrawPoint();

    auto& v0 =
        gDebugRenderContext->vertexBuffer[gDebugRenderContext->vertexCount++];

    *(Vector3*)(&v0.position[0]) = point->position;
    v0.color = point->color;
}

void DebugPass::flushDrawPoint()
{
    if (gDebugRenderContext->vertexCount == 0)
        return;

    m_vbo->SetData(gDebugRenderContext->vertexBuffer, gDebugRenderContext->vertexCount * sizeof(DebugVertex));

    glPointSize(3.0f);

    glDrawArrays(GL_POINTS, 0, gDebugRenderContext->vertexCount);

    gDebugRenderContext->vertexCount = 0;
}


