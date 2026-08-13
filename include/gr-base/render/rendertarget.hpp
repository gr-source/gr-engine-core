#pragma once

#include "gr-base/core/assetPtr.hpp"

#include <gr-render/texture.hpp>

struct RenderTargetDesc
{
    int width = 512;
    int height = 512;

    int colorFormat = TEXTURE_FORMAT_RGBA16F;
    int depthFormat = TEXTURE_FORMAT_DEPTH;
};

class RenderTarget
{
public:
    RenderTarget(const RenderTargetDesc& desc);
    ~RenderTarget();

    void bind();

    void unbind();

    void resize(int width, int height);

    AssetPtr<gr::texture> GetTextureColor() const;

private:
    AssetPtr<gr::texture> textureColor;

    AssetPtr<gr::texture> textureDepth;

    uint32_t framebuffer;
};


