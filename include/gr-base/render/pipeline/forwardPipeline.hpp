#pragma once

#include "gr-base/render/pipeline/renderPipeline.hpp"
#include "gr-base/render/pipeline/renderGraph.hpp"

#include "gr-base/core/assetPtr.hpp"

#include <gr-render/default.hpp>

class ForwardPipeline : public IRenderPipeline
{
public:
    AssetPtr<gr::texture> GetTextureOutput() const;

private:
    AssetPtr<gr::texture> m_hdr;

    AssetPtr<gr::texture> m_ldr;

    AssetPtr<gr::texture> m_depth;

    // AssetPtr<gr::texture> m_output;

    RenderGraph graph;

    void Initialize() override;

    void Execute() override;
};


