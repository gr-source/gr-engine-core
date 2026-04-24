#include "gr-base/asset/meshasset.hpp"

#include "gr-base/core/file/stream.hpp"

bool MeshAsset::serialize(Stream* stream) const
{
    if (!stream->write(m_name))
        return false;

    uint64_t verticecount = static_cast<uint64_t>(vertices.size());
    if (!stream->write(verticecount))
        return false;

    if (!stream->write(vertices.data(), verticecount * sizeof(Vertex3D)))
        return false;

    uint64_t subcount = static_cast<uint64_t>(submeshes.size());
    if (!stream->write(subcount))
        return false;

    for (auto& sub : submeshes)
    {
        if (!stream->write(sub.materialName))
            return false;

        uint64_t indicescount = static_cast<uint64_t>(sub.indices.size());
        if (!stream->write(indicescount))
            return false;

        if (!stream->write(sub.indices.data(), indicescount * sizeof(uint32_t)))
            return false;
    }

    return true;
}

bool MeshAsset::deserialize(Stream* stream)
{
    if (!stream->read(m_name))
        return false;

    uint64_t verticecount = 0;
    if (!stream->read(verticecount))
        return false;

    vertices.resize(verticecount, {});
    if (!stream->read(vertices.data(), verticecount * sizeof(Vertex3D)))
        return false;

    uint64_t subcount = 0;
    if (!stream->read(subcount))
        return false;

    submeshes.resize(subcount, {});
    for (auto& sub : submeshes)
    {
        if (!stream->read(sub.materialName))
            return false;

        uint64_t indicescount = 0;
        if (!stream->read(indicescount))
            return false;

        sub.indices.resize(indicescount, 0);
        if (!stream->read(sub.indices.data(), indicescount * sizeof(uint32_t)))
            return false;
    }

    return true;
}

