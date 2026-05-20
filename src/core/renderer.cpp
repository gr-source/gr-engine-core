#include "gr-base/core/renderer.hpp"

#include "gr-base/core/debugdraw.hpp"
#include "gr-base/core/rendertarget.hpp"
#include <gr-render/gRender.h>

 RenderTarget* Renderer::backbuffer = nullptr;

void Renderer::Initialize()
{
    if (backbuffer == nullptr)
        backbuffer = new RenderTarget(1024, 1024);
}

void Renderer::Shutdown()
{
    if (backbuffer != nullptr)
        delete backbuffer;
    backbuffer = nullptr;
}

struct Data
{
    RenderTarget* rendertarget = nullptr;
} data;

void Renderer::Process()
{
    if (data.rendertarget == nullptr)
        return;

    data.rendertarget->bind();

    gr::gRender::SetRenderState(gr::GR_CULL, gr::GR_BACK);
    gr::gRender::SetEnable(GR_DEPTH, true);

    gr::gRender::SetBackgroundColor(Color{0.5f, 0.5f, 0.5f, 1.0f});
    gr::gRender::SetRenderState(gr::GR_BACKGROUND, gr::GR_DEPTH_BUFFER | gr::GR_COLOR_BUFFER);


    gr::gRender::SetRenderState(gr::GR_DEPTH_FUNC, gr::GR_DEPTH_LESS);
    // RenderSystem::Renderer();
    // RenderManager::Render();

    gr::gRender::SetRenderState(gr::GR_DEPTH_FUNC, gr::GR_DEPTH_LEQUAL);
    DebugDraw::Render();

    // gr::gRender::SetRenderState(gr::GR_DEPTH_FUNC, gr::GR_DEPTH_LEQUAL);
    // m_skybox.render(camera);

    data.rendertarget->unbind();
}

void Renderer::SetRenderTarget(RenderTarget* rt)
{
    data.rendertarget = rt;
}




