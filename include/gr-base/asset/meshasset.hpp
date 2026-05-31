#pragma once

#include "gr-base/core/default.hpp"
#include "gr-base/core/geometry/vertex3D.hpp"

#include "gr-base/core/file/default.hpp"

#include <vector>

struct SubMeshAsset
{
    std::vector<uint32_t> indices;

    std::string materialName;
};

struct MeshAsset
{
    std::vector<SubMeshAsset> submeshes;

    std::vector<Vertex3D> vertices;

    std::string name;

    AssetHash hash = INVALID_HASH64;

    bool serialize(Stream* stream) const;

    bool deserialize(Stream* stream);
};


