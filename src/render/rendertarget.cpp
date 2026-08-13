#include "gr-base/render/rendertarget.hpp"

#include <gr-render/gl.h>

void RenderTarget::bind()
{
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void RenderTarget::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::resize(int w, int h)
{
    if (w == width && h == height)
        return;
    width = w;
    height = h;

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



