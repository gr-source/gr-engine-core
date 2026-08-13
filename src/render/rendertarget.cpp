#include "gr-base/render/rendertarget.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include <gr-render/gl.h>

RenderTarget::RenderTarget(const RenderTargetDesc& desc) :
    framebuffer(0)
{
    textureColor =
        AssetsManager::CreateTexture();

    textureColor->width =
        desc.width;
    textureColor->height =
        desc.height;

    textureColor->type =
        TEXTURE_TYPE_2D;
    textureColor->filter =
        TEXTURE_FILTER_LINEAR;
    textureColor->wrap =
        TEXTURE_WRAP_REPEAT;
    textureColor->format =
        desc.colorFormat;

    textureColor->bind();
    textureColor->applyTextureSettings();
    textureColor->applyTextureBuffer(nullptr);
    textureColor->unbind();

    textureDepth =
        AssetsManager::CreateTexture();

    textureDepth->width =
        desc.width;
    textureDepth->height =
        desc.height;

    textureDepth->type =
        TEXTURE_TYPE_2D;
    textureDepth->filter =
        TEXTURE_FILTER_NEAREST;
    textureDepth->wrap =
        TEXTURE_WRAP_CLAMP;
    textureDepth->format =
        desc.depthFormat;

    textureDepth->bind();
    textureDepth->applyTextureSettings();
    textureDepth->applyTextureBuffer(nullptr);
    textureDepth->unbind();

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColor->id, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepth->id, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

RenderTarget::~RenderTarget()
{
    if (textureColor.get() != nullptr)
        AssetsManager::DestroyTexture(textureColor.handle);

    if (textureDepth.get() != nullptr)
        AssetsManager::DestroyTexture(textureDepth.handle);

    if (framebuffer)
        glDeleteFramebuffers(1, &framebuffer);
}

void RenderTarget::bind()
{
    // glViewport(0, 0, m_width, m_height);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // glEnable(GL_FRAMEBUFFER_SRGB);
}

void RenderTarget::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glDisable(GL_FRAMEBUFFER_SRGB);
}

void RenderTarget::resize(int width, int height)
{
    textureColor->bind();
    textureColor->width = width;
    textureColor->height = height;
    textureColor->applyTextureBuffer(nullptr);
    textureColor->unbind();

    textureDepth->bind();
    textureDepth->width = width;
    textureDepth->height = height;
    textureDepth->applyTextureBuffer(nullptr);
    textureDepth->unbind();
}

AssetPtr<gr::texture> RenderTarget::GetTextureColor() const
{
    return textureColor;
}



