#include "gr-base/core/rendertarget.hpp"

#include <gr-render/gl.h>

RenderTarget::RenderTarget(std::uint32_t width, std::uint32_t height) : textureColor(0), textureDepth(0), framebuffer(0), width(width), height(height), renderbufferDepth(0), bvalid(false)
{}

void RenderTarget::init()
{
    if (bvalid)
        return;

    glGenTextures(1, &textureColor);
    glBindTexture(GL_TEXTURE_2D, textureColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &textureDepth);
    glBindTexture(GL_TEXTURE_2D, textureDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColor, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepth, 0);

    /*glGenRenderbuffers(1, &renderbufferDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbufferDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    */
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbufferDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Erro ao configurar o framebuffer!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    bvalid = true;
}

void RenderTarget::shutdown()
{
    if (textureColor)
    {
        glDeleteTextures(1, &textureColor);
    }

    if (textureDepth)
    {
        glDeleteTextures(1, &textureDepth);
    }

    if (framebuffer)
    {
        glDeleteFramebuffers(1, &framebuffer);
    }

    if (renderbufferDepth)
    {
        glDeleteRenderbuffers(1, &renderbufferDepth);
    }

    bvalid = false;
}

bool RenderTarget::is_valid() const
{
    return bvalid;
}

RenderTarget::~RenderTarget()
{
    shutdown();
}

void RenderTarget::bind()
{
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void RenderTarget::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::resize(std::uint32_t w, std::uint32_t h)
{
    if (width == w && height == h)
        return;
    width = w;
    height = h;

    glBindTexture(GL_TEXTURE_2D, textureColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, textureDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}

std::uint32_t RenderTarget::getTextureID() const
{
    return textureColor;
}



