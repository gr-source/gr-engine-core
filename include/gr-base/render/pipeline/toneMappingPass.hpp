#pragma once

#include "gr-base/render/pipeline/renderPass.hpp"

class ToneMappingPass : public IRenderPass
{
public:
    ToneMappingPass(TextureRef hdr, TextureRef ldr);

private:
    TextureRef m_hdr;
    TextureRef m_ldr;

    gr::shader* shader =
        nullptr;

    void Setup(RenderGraphBuild& build) override;

    void Execute() override;
};

