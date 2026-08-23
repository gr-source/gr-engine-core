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

void ForwardPipeline::Render(CameraRenderData& camera)
{
    graph.Execute();
}

