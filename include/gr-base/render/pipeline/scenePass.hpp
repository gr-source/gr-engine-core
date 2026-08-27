#include "gr-base/render/pipeline/renderPass.hpp"

class ScenePass : public IRenderPass
{
public:
    ScenePass(TextureRef hdr, TextureRef depth);

private:
    TextureRef m_hdr;
    
    TextureRef m_depth;

    void Setup(RenderGraphBuild& build) override;

    void Execute() override;
};

