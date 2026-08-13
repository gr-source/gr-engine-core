#pragma once

#include "gr-base/render/default.hpp"
#include "gr-base/render/pipeline/renderGraph.hpp"
#include "gr-base/render/pipeline/renderPipeline.hpp"

class ForwardPipeline : public IRenderPipeline
{
public:

    RenderTargetID GetOutput() const;

private:
    RenderTargetID rendertarget =
        InvalidRenderTarget;

    RenderGraph graph;

    void Initialize() override;

    void Execute() override;
};


