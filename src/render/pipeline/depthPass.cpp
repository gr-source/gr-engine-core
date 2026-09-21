#include "gr-base/render/pipeline/depthPass.hpp"

#include "gr-base/core/renderer.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include <GL/gl.h>
#include <gr-render/texture.hpp>
#include <gr-render/shader.hpp>

#include <iostream>

DepthPass::DepthPass(TextureRef depth, TextureRef ldr) :
    m_depth(depth),
    m_ldr(ldr)
{}

void DepthPass::Setup(RenderGraphBuild& build)
{
    build.Write(m_ldr);
    build.Read(m_depth);

    build.AddColorAttachment(m_ldr);

    clearColor = false;
    clearDepth = true;

    try
    {

    shader = AssetsManager::LoadShader("project://shader/depth")->Get(ShaderVariantFlags_None);
    } catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void DepthPass::Execute()
{
    shader->bind();

    glActiveTexture(GL_TEXTURE0);
    m_depth->bind();
    shader->set_uniform("u_texture", 0);

    Renderer::DrawFullScreenQuad();

    shader->unbind();


    glActiveTexture(GL_TEXTURE0);
    m_depth->unbind();
}

