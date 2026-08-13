#include "gr-base/render/pipeline/renderGraph.hpp"

#include "gr-base/render/pipeline/renderPass.hpp"

#include <gr-math/color.h>
#include <gr-render/gRender.h>
#include <gr-render/texture.hpp>
#include <gr-render/gl.h>

void RenderGraph::Execute()
{
    uint32_t lastFrameBuffer = -1;
    for (auto&& [target, pass] : passes)
    {
        if (lastFrameBuffer != target.framebuffer)
        {
            target.bind();

            gr::gRender::SetRenderState(gr::GR_CULL, gr::GR_BACK);
            gr::gRender::SetEnable(GR_DEPTH, true);
            gr::gRender::SetEnable(GR_CULL_FACE, true);
            // gr::gRender::SetEnable(GR_FRAMEBUFFER_SRGB, true);

            // gr::gRender::SetViewport(viewport.data);
            gr::gRender::SetBackgroundColor(Color{0.5f, 0.5f, 0.5f, 1.0f}.data);
            gr::gRender::SetRenderState(gr::GR_BACKGROUND, gr::GR_DEPTH_BUFFER | gr::GR_COLOR_BUFFER);
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            lastFrameBuffer = target.framebuffer;
        }

        pass->Execute();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderGraph::compile()
{
}

RenderTarget RenderGraph::Create(AssetPtr<gr::texture> color, AssetPtr<gr::texture> depth)
{
    uint32_t framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color->id, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth->id, 0);

    const GLenum drawBuffers[] =
    {
        GL_COLOR_ATTACHMENT0
    };

    glDrawBuffers(1, drawBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return
    {
        .textureColor = color,
        .textureDepth = depth,
        .framebuffer = framebuffer,
        .width = (int)color->width,
        .height = (int)color->height
    };

}

void RenderGraph::Set(std::shared_ptr<IRenderPass> pass)
{
    RenderGraphBuild builder;
    pass->Setup(builder);

    Handle sign =
        handle_create(handle_get_first(builder.color), handle_get_first(builder.depth));

    if (m_cacheRenderTarget.find(sign) == m_cacheRenderTarget.end())
    {
        RenderTarget rt =
            Create({builder.color}, {builder.depth});

        m_cacheRenderTarget[sign] = rt;
    }

    passes.emplace_back(m_cacheRenderTarget[sign], pass);
}

