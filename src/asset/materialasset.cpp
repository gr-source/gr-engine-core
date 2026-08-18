#include "gr-base/asset/materialasset.hpp"

#include "gr-base/file/stream.hpp"

#include <cstring>

void MaterialAsset::Set(const std::string& name, const void* data, size_t size)
{
    auto& cache = overrides[name];
    cache.resize(size, 0);

    memcpy(cache.data(), data, size);
}


void MaterialAsset::SetTexture(const std::string& name, AssetID assetID)
{
    textures[name] = assetID;
}

bool MaterialAsset::serialize(Stream* stream) const
{
    if (!stream->write(name))
        return false;

    if (!stream->write(shader))
        return false;

    uint64_t count = static_cast<uint64_t>(overrides.size());
    if (!stream->write(count))
        return false;

    for (auto&& [name, buffer] : overrides)
    {
        if (!stream->write(name))
            return false;

        uint64_t length = static_cast<uint64_t>(buffer.size());
        if (!stream->write(length))
            return false;

        if (!stream->write(buffer.data(), length))
            return false;
    }

    uint64_t textureCount =
        static_cast<uint64_t>(textures.size());

    if (!stream->write(textureCount))
        return false;

    for (auto&& [name, assetID] : textures)
    {
        if (!stream->write(name))
            break;

        if (!stream->write(assetID))
            break;
    }

    return true;
}

bool MaterialAsset::deserialize(Stream* stream)
{
    if (!stream->read(name))
        return false;

    if (!stream->read(shader))
        return false;

    uint64_t count = 0;
    if (!stream->read(count))
        return false;

    for (uint64_t i=0;i<count;i++)
    {
        std::string name;
        if (!stream->read(name))
            return false;

        uint64_t length = 0;
        if (!stream->read(length))
            return false;

        std::vector<uint8_t> buffer(length, 0);
        if (!stream->read(buffer.data(), length))
            return false;

        overrides.emplace(name, buffer);
    }

    uint64_t textureCount =
        0;

    if (!stream->read(textureCount))
        return false;

    for (uint64_t i=0;i<textureCount;i++)
    {
        std::string name;
        if (!stream->read(name))
            break;

        AssetID assetID;
        if (!stream->read(assetID))
            break;

        SetTexture(name, assetID);
    }

    return true;
}

