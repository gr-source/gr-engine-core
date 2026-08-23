#include "gr-base/asset/shaderasset.hpp"

#include "gr-base/file/stream.hpp"

#include <cstdint>

bool ShaderAsset::serialize(Stream* stream) const
{
    if (!stream->write(name))
        return false;

    if (!stream->write(fragment))
        return false;

    if (!stream->write(vertex))
        return false;

    if (!stream->write(domain))
        return false;
    return true;
}

bool ShaderAsset::deserialize(Stream* stream)
{
    if (!stream->read(name))
        return false;

    if (!stream->read(fragment))
        return false;

    if (!stream->read(vertex))
        return false;

    if (!stream->read(domain))
        return false;
    return true;
}

