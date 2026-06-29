#include "gr-base/asset/textureAsset.hpp"

#include "gr-base/core/file/stream.hpp"

bool TextureAsset::serialize(Stream* stream) const
{
    if (!stream->write(width))
        return false;

    if (!stream->write(height))
        return false;

    if (!stream->write(channels))
        return false;

    if (!stream->write(hdr))
        return false;

    uint64_t size = static_cast<uint64_t>(raw.size());
    if (!stream->write(size))
        return false;

    if (!stream->write(raw.data(), size))
        return false;

    return true;
}

bool TextureAsset::deserialize(Stream* stream)
{
    if (!stream->read(width))
        return false;

    if (!stream->read(height))
        return false;

    if (!stream->read(channels))
        return false;

    if (!stream->read(hdr))
        return false;

    uint64_t size = 0;
    if (!stream->read(size))
        return false;

    raw.resize(size, 0);
    if (!stream->read(raw.data(), size))
        return false;

    return true;
}


