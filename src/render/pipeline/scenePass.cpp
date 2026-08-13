#include "gr-base/render/pipeline/scenePass.hpp"
#include "gr-base/core/renderer.hpp"

ScenePass::ScenePass(
        AssetPtr<gr::texture> hdr,
        AssetPtr<gr::texture> depth) :
    m_hdr(hdr),
    m_depth(depth)
{}

void ScenePass::Setup(RenderGraphBuild& build)
{
    build.color = m_hdr.handle;
    build.depth = m_depth.handle;
}

void ScenePass::Execute()
{
    Renderer::Render();
}
