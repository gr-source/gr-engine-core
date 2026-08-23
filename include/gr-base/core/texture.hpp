#pragma once

#include <gr-render/texture.hpp>

struct TextureDesc
{
    uint32_t width  = 512;
    uint32_t height = 512;

    uint32_t filter = TEXTURE_FILTER_LINEAR;
    uint32_t format = TEXTURE_FORMAT_RGB;
    uint32_t wrap = TEXTURE_WRAP_REPEAT;
    uint32_t type = TEXTURE_TYPE_2D;

    void* pixels = nullptr;
};

