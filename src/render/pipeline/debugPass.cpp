#include "gr-base/render/pipeline/debugPass.hpp"

#include <gr-base/debug/debugDraw.hpp>

#include <gr-render/gRender.h>
#include <gr-render/gl.h>

DebugPass::DebugPass(
        AssetPtr<gr::texture> hdr,
        AssetPtr<gr::texture> depth) :
    m_hdr(hdr),
    m_depth(depth)
{}

void DebugPass::Setup(RenderGraphBuild& build)
{
    build.write.push_back(m_hdr.handle);
    build.read.push_back(m_depth.handle);

    build.AddColorAttachment(m_hdr);
    build.SetDepthAttachment(m_depth);
}

void DebugPass::Execute()
{
    // gr::gRender::SetEnable(GR_DEPTH, false);

    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    gr::gRender::SetDepthFunc(GR_DEPTH_FUNC_LEQUAL);
    DebugDraw::Renderer();
}
