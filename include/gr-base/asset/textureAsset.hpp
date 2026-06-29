#pragma once

#include "gr-base/core/file/default.hpp"

#include <cstdint>
#include <vector>

struct TextureAsset
{
    uint32_t width = 0;
    uint32_t height = 0;

    uint32_t channels = 0;

    bool hdr = false;

    std::vector<uint8_t> raw;

    bool serialize(Stream* stream) const;

    bool deserialize(Stream* stream);
};

