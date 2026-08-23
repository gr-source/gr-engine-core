#include "gr-base/render/pipeline/forwardPipeline.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/render/pipeline/debugPass.hpp"
#include "gr-base/render/pipeline/scenePass.hpp"
#include "gr-base/render/pipeline/toneMappingPass.hpp"

#include <gr-render/texture.hpp>

AssetPtr<gr::texture> ForwardPipeline::GetTextureOutput() const
{
    return m_ldr;
}

void ForwardPipeline::Initialize()
{
    m_hdr =
        AssetsManager::CreateTexture({
            .width = 512,
            .height = 512,

            .filter =
                TEXTURE_FILTER_LINEAR,
            .format =
                TEXTURE_FORMAT_RGBA16F,
            .wrap =
                TEXTURE_WRAP_REPEAT,
            .type =
                TEXTURE_TYPE_2D,
            });


    m_ldr =
        AssetsManager::CreateTexture({
            .width = 512,
            .height = 512,

            .filter =
                TEXTURE_FILTER_LINEAR,
            .format =
                TEXTURE_FORMAT_RGBA8,
            .wrap =
                TEXTURE_WRAP_REPEAT,
            .type =
                TEXTURE_TYPE_2D,
            });

    m_depth =
        AssetsManager::CreateTexture({
            .width = 512,
            .height = 512,

            .filter =
                TEXTURE_FILTER_LINEAR,
            .format =
                TEXTURE_FORMAT_DEPTH,
            .wrap =
                TEXTURE_WRAP_REPEAT,
            .type =
                TEXTURE_TYPE_2D,
            });


    graph.add<ScenePass>(m_hdr, m_depth);
    graph.add<DebugPass>(m_hdr, m_depth);
    graph.add<ToneMappingPass>(m_hdr, m_ldr);

    graph.compile();
}

#include <gr-render/gl.h>

void ForwardPipeline::Render(CameraRenderData& camera)
{
    auto& camViewport = camera.viewport;
    if (camViewport.w != viewport.w  || camViewport.h != viewport.h)
    {
        // hdr
        m_hdr->width = camViewport.w;
        m_hdr->height = camViewport.h;

        m_hdr->bind();
        m_hdr->applyTextureBuffer(nullptr);
        m_hdr->unbind();

        // ldr
        m_ldr->width = camViewport.w;
        m_ldr->height = camViewport.h;

        m_ldr->bind();
        m_ldr->applyTextureBuffer(nullptr);
        m_ldr->unbind();

        // depth
        m_depth->width = camViewport.w;
        m_depth->height = camViewport.h;

        m_depth->bind();
        m_depth->applyTextureBuffer(nullptr);
        m_depth->unbind();

        viewport = camViewport;
    }


    glViewport(viewport.x, viewport.y, viewport.w, viewport.h);

    // if (viewport.w !=)
    graph.Execute();
}

