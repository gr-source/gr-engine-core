#include "gr-base/managers/renderTargetManager.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include <gr-render/gl.h>

#include <new>

RenderTargetManager* RenderTargetManager::gRenderTargetManager = nullptr;

// public
void RenderTargetManager::Initialize()
{
    if (gRenderTargetManager == nullptr)
        gRenderTargetManager = new RenderTargetManager();
}

void RenderTargetManager::Shutdown()
{
    if (gRenderTargetManager != nullptr)
        delete gRenderTargetManager;

    gRenderTargetManager = nullptr;
}

RenderTargetID RenderTargetManager::Create(const RenderTargetDesc& desc)
{
    return gRenderTargetManager->create(desc);
}

RenderTarget* RenderTargetManager::Get(RenderTargetID id)
{
    return gRenderTargetManager->sparseSet.get(id.value);
}

// private
RenderTargetID RenderTargetManager::create(const RenderTargetDesc& desc)
{
    AssetPtr<gr::texture> textureColor =
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

    AssetPtr<gr::texture> textureDepth =
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

    uint32_t framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColor->id, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepth->id, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    SparseID id =
        sparseSet.emplace();

    auto renderTarget = sparseSet.get(id);
    renderTarget->textureColor = textureColor;
    renderTarget->textureDepth = textureDepth;
    renderTarget->framebuffer = framebuffer;

    return { id };
}


