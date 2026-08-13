#include "gr-base/render/pipeline/renderPass.hpp"

class ScenePass : public IRenderPass
{
public:
    ScenePass(AssetPtr<gr::texture> hdr, AssetPtr<gr::texture> depth);

private:
    AssetPtr<gr::texture> m_hdr;
    
    AssetPtr<gr::texture> m_depth;

    void Setup(RenderGraphBuild& build) override;

    void Execute() override;
};

