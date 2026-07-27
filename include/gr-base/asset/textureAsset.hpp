#pragma once

#include "gr-base/asset/iasset.hpp"

#include "gr-base/core/file/default.hpp"

#include <cstdint>
#include <vector>

struct TextureAsset : IAsset
{
    uint32_t width = 0;
    uint32_t height = 0;

    uint32_t channels = 0;

    bool hdr = false;

    std::vector<uint8_t> raw;

    bool serialize(Stream* stream) const override;

    bool deserialize(Stream* stream) override;
};

