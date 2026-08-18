#include "gr-base/render/pipeline/toneMappingPass.hpp"

#include "gr-base/core/renderer.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/assetDB.hpp"

#include <GL/gl.h>
#include <gr-render/texture.hpp>
#include <gr-render/shader.hpp>

ToneMappingPass::ToneMappingPass(AssetPtr<gr::texture> hdr, AssetPtr<gr::texture> ldr) :
    m_hdr(hdr),
    m_ldr(ldr)
{}

void ToneMappingPass::Setup(RenderGraphBuild& build)
{
    build.write.push_back(m_ldr.handle);
    build.read.push_back(m_hdr.handle);

    build.AddColorAttachment(m_ldr);

    clearColor = true;
    clearDepth = false;

    shader = AssetsManager::LoadShader(AssetDB::FindByName("engine://shader/screen"))->Get(ShaderVariantFlags_none);
}

void ToneMappingPass::Execute()
{
    shader->bind();

    glActiveTexture(GL_TEXTURE0);
    m_hdr->bind();
    shader->set_uniform("u_texture", 0);

    Renderer::DrawFullScreenQuad();

    shader->unbind();


    // glActiveTexture(GL_TEXTURE0);
    // m_hdr->unbind();
}

