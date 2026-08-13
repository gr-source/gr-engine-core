#include "gr-base/render/pipeline/forwardPipeline.hpp"

#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/renderTargetManager.hpp"
#include "gr-base/render/pipeline/debugPass.hpp"
#include "gr-base/render/pipeline/scenePass.hpp"
#include <gr-math/color.h>

AssetPtr<gr::texture> ForwardPipeline::GetTextureOutput() const
{
    return m_hdr;
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

    graph.compile();
}

#include <gr-render/gRender.h>

void ForwardPipeline::Execute()
{
    graph.Execute();
}

