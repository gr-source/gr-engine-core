#include "gr-base/asset/meshasset.hpp"

#include "gr-base/core/file/stream.hpp"

bool MeshAsset::serialize(Stream* stream) const
{
    if (!stream->write(name))
        return false;

    if (!stream->write(hash))
        return false;

    {
        uint64_t verticecount = static_cast<uint64_t>(vertices.size());
        if (!stream->write(verticecount))
            return false;

        if (!stream->write(vertices.data(), verticecount * sizeof(Vertex3D)))
            return false;
    }

    {
        uint64_t indexCount = static_cast<uint64_t>(indices.size());
        if (!stream->write(indexCount))
            return false;

        if (!stream->write(indices.data(), indexCount * sizeof(uint32_t)))
            return false;
    }

    uint64_t subcount = static_cast<uint64_t>(submeshes.size());
    if (!stream->write(subcount))
        return false;

    for (auto& sub : submeshes)
    {
        if (!stream->write(sub.materialName))
            return false;

        if (!stream->write(sub.indexCount))
            return false;

        if (!stream->write(sub.indexOffset))
            return false;
    }

    return true;
}

bool MeshAsset::deserialize(Stream* stream)
{
    if (!stream->read(name))
        return false;

    if (!stream->read(hash))
        return false;

    {
        uint64_t verticecount = 0;
        if (!stream->read(verticecount))
            return false;

        vertices.resize(verticecount, {});
        if (!stream->read(vertices.data(), verticecount * sizeof(Vertex3D)))
            return false;
    }

    {
        uint64_t indicesCount = 0;
        if (!stream->read(indicesCount))
            return false;

        indices.resize(indicesCount, 0);
        if (!stream->read(indices.data(), indicesCount * sizeof(uint32_t)))
            return false;
    }

    uint64_t subcount = 0;
    if (!stream->read(subcount))
        return false;

    submeshes.resize(subcount, {});
    for (auto& sub : submeshes)
    {
        if (!stream->read(sub.materialName))
            return false;

        if (!stream->read(sub.indexCount))
            return false;

        if (!stream->read(sub.indexOffset))
            return false;
    }

    return true;
}

