#include "gr-base/render/pipeline/scenePass.hpp"
#include "gr-base/core/renderer.hpp"
#include <gr-render/gRender.h>

ScenePass::ScenePass(
        TextureRef hdr,
        TextureRef depth) :
    m_hdr(hdr),
    m_depth(depth)
{}

void ScenePass::Setup(RenderGraphBuild& build)
{
    build.Write(m_hdr);
    build.Read(m_depth);

    build.AddColorAttachment(m_hdr);
    build.SetDepthAttachment(m_depth);

    clearColor = true;
    clearDepth = true;
}

#include <gr-render/gl.h>

void ScenePass::Execute()
{
    gr::gRender::SetEnable(GR_DEPTH, true);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    Renderer::Render();

    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
