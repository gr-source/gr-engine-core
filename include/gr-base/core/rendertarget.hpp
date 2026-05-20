#pragma once

#include <cstdint>

class RenderTarget
{
public:
    RenderTarget(int width, int height);
    ~RenderTarget();

    void init();

    void shutdown();

    bool is_valid() const;

    void bind();

    void unbind();

    void resize(int width, int height);

    uint32_t GetTextureID() const;

    inline int GetWidth() const
    {
        return m_width;
    }

    inline int GetHeight() const
    {
        return m_height;
    }

private:
    uint32_t textureColor;

    uint32_t textureDepth;

    uint32_t framebuffer;

    uint32_t renderbufferDepth;

    int m_width;

    int m_height;

    bool bvalid;
};


