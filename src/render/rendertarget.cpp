#include "gr-base/render/rendertarget.hpp"

#include <gr-render/gl.h>

void RenderTarget::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void RenderTarget::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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



