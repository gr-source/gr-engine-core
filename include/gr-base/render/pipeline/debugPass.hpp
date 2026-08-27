#pragma once

#include "gr-base/render/pipeline/renderPass.hpp"

class DebugPass : public IRenderPass
{
public:
    DebugPass(TextureRef hdr, TextureRef depth);

private:
    TextureRef m_hdr;

    TextureRef m_depth;

    void Setup(RenderGraphBuild& build) override;

    void Execute() override;
};

