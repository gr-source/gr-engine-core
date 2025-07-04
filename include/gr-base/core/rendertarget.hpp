#pragma once

#include <cstdint>

class RenderTarget
{
public:
    RenderTarget(std::uint32_t width, std::uint32_t height);
    ~RenderTarget();

    void init();

    void shutdown();

    bool is_valid() const;

    void bind();

    void unbind();

    void resize(std::uint32_t width, std::uint32_t height);

    std::uint32_t getTextureID() const;

private:
    std::uint32_t textureColor;

    std::uint32_t textureDepth;

    std::uint32_t framebuffer;

    std::uint32_t renderbufferDepth;

    std::uint32_t width;

    std::uint32_t height;

    bool bvalid;
};


