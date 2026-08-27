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
    build.write.push_back(m_hdr);
    build.read.push_back(m_depth);

    build.AddColorAttachment(m_hdr);
    build.SetDepthAttachment(m_depth);

    clearColor = true;
    clearDepth = true;
}

void ScenePass::Execute()
{
    gr::gRender::SetEnable(GR_DEPTH, true);

    Renderer::Render();
}
