#include "gr-base/asset/modelasset.hpp"

#include "gr-base/file/stream.hpp"

#include <cstring>

void ModelAsset::AddComponent(Hash64 hash, const void* data, size_t size)
{
    std::vector<uint8_t> buffer(size, 0);
    memcpy(buffer.data(), data, size);

    componentBuffer.emplace(hash, buffer);
}

bool ModelAsset::serialize(Stream* stream) const
{
    if (!stream->write(name))
        return false;

    if (!stream->write(mesh))
        return false;

    {
        uint64_t count =
            static_cast<uint64_t>(materials.size());

        if (!stream->write(count))
            return false;

        if (!stream->write(materials.data(), count * sizeof(AssetID)))
            return false;
    }

    uint64_t componentCount = static_cast<uint64_t>(componentBuffer.size());
    if (!stream->write(componentCount))
        return false;

    for (auto &&[hash, buffer] : componentBuffer)
    {
        if (!stream->write(hash))
            return false;

        uint64_t length = static_cast<uint64_t>(buffer.size());
        if (!stream->write(length))
            return false;

        if (!stream->write(buffer.data(), length))
            return false;
    }

    uint64_t count = static_cast<uint64_t>(children.size());
    if (!stream->write(count))
        return false;

    for (auto& child : children)
    {
        if (!child.serialize(stream))
            return false;
    }

    return true;
}

bool ModelAsset::deserialize(Stream* stream)
{
    if (!stream->read(name))
        return false;

    if (!stream->read(mesh))
        return false;

    {
        uint64_t count;
        if (!stream->read(count))
            return false;

        materials.resize(count);
        if (!stream->read(materials.data(), count * sizeof(AssetID)))
            return false;
    }

    uint64_t componentCount = 0;
    if (!stream->read(componentCount))
        return false;

    for (uint64_t i=0;i<componentCount;i++)
    {
        uint64_t hash;
        if (!stream->read(hash))
            return false;

        uint64_t length = 0;
        if (!stream->read(length))
            return false;

        std::vector<uint8_t> buffer(length, 0);
        if (!stream->read(buffer.data(), length))
            return false;

        componentBuffer.emplace(hash, buffer);
    }

    uint64_t count = 0;
    if (!stream->read(count))
        return false;

    for (uint64_t i=0;i<count;i++)
    {
        auto& child = children.emplace_back();

        if (!child.deserialize(stream))
            return false;
    }

    return true;
}

