#include "gr-base/render/pipeline/forwardPipeline.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/render/pipeline/debugPass.hpp"
#include "gr-base/render/pipeline/scenePass.hpp"
#include "gr-base/render/pipeline/toneMappingPass.hpp"

AssetPtr<gr::texture> ForwardPipeline::GetTextureOutput() const
{
    return m_ldr;
}

void ForwardPipeline::Initialize()
{
    m_hdr =
        AssetsManager::CreateTexture();

    m_hdr->width =
        512;
    m_hdr->height =
        512;

    m_hdr->type =
        TEXTURE_TYPE_2D;
    m_hdr->filter =
        TEXTURE_FILTER_LINEAR;
    m_hdr->wrap =
        TEXTURE_WRAP_REPEAT;
    m_hdr->format =
        TEXTURE_FORMAT_RGBA16F;

    m_hdr->bind();
    m_hdr->applyTextureSettings();
    m_hdr->applyTextureBuffer(nullptr);
    m_hdr->unbind();

    m_ldr =
        AssetsManager::CreateTexture();

    m_ldr->width =
        512;
    m_ldr->height =
        512;

    m_ldr->type =
        TEXTURE_TYPE_2D;
    m_ldr->filter =
        TEXTURE_FILTER_LINEAR;
    m_ldr->wrap =
        TEXTURE_WRAP_REPEAT;
    m_ldr->format =
        TEXTURE_FORMAT_RGBA;

    m_ldr->bind();
    m_ldr->applyTextureSettings();
    m_ldr->applyTextureBuffer(nullptr);
    m_ldr->unbind();

    m_depth =
        AssetsManager::CreateTexture();

    m_depth->width =
        512;
    m_depth->height =
        512;

    m_depth->type =
        TEXTURE_TYPE_2D;
    m_depth->filter =
        TEXTURE_FILTER_NEAREST;
    m_depth->wrap =
        TEXTURE_WRAP_CLAMP;
    m_depth->format =
        TEXTURE_FORMAT_DEPTH;

    m_depth->bind();
    m_depth->applyTextureSettings();
    m_depth->applyTextureBuffer(nullptr);
    m_depth->unbind();

    graph.add<ScenePass>(m_hdr, m_depth);
    graph.add<DebugPass>(m_hdr, m_depth);
    graph.add<ToneMappingPass>(m_hdr, m_ldr);

    graph.compile();
}

void ForwardPipeline::Execute()
{
    graph.Execute();
}

