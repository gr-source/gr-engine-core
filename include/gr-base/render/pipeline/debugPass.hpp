#pragma once

#include "gr-base/render/pipeline/renderPass.hpp"

class DebugPass : public IRenderPass
{
public:
    DebugPass(AssetPtr<gr::texture> hdr, AssetPtr<gr::texture> depth);

private:
    AssetPtr<gr::texture> m_hdr;

    AssetPtr<gr::texture> m_depth;

    void Setup(RenderGraphBuild& build) override;

    void Execute() override;
};

