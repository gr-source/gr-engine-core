#pragma once

#include "gr-base/render/pipeline/renderPass.hpp"

class ToneMappingPass : public IRenderPass
{
public:
    ToneMappingPass(AssetPtr<gr::texture> hdr, AssetPtr<gr::texture> ldr);

private:
    AssetPtr<gr::texture> m_hdr;
    AssetPtr<gr::texture> m_ldr;

    gr::shader* shader =
        nullptr;

    void Setup(RenderGraphBuild& build) override;

    void Execute() override;
};

