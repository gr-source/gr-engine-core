#pragma once

#include "gr-base/render/pipeline/renderPass.hpp"

class DepthPass : public IRenderPass
{
public:
    DepthPass(TextureRef depth, TextureRef ldr);

private:
    TextureRef m_depth;
    TextureRef m_ldr;

    gr::shader* shader =
        nullptr;

    void Setup(RenderGraphBuild& build) override;

    void Execute() override;
};

