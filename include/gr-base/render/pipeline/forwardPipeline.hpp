#pragma once

#include "gr-base/render/pipeline/ipipeline.hpp"
#include "gr-base/render/pipeline/renderGraph.hpp"

#include "gr-base/core/assetPtr.hpp"

#include <gr-render/default.hpp>

class ForwardPipeline : public IPipeline
{
public:
    TextureRef GetTextureOutput() const;

private:
    TextureRef m_hdr;

    TextureRef m_ldr;

    TextureRef m_depth;

    RenderGraph graph;

    Rect viewport = { 0.0f, 0.0f, 512.0f, 512.0f };

    void Initialize() override;

    void Render(CameraRenderData& camera) override;

    void resize(uint32_t width, uint32_t height);
};


