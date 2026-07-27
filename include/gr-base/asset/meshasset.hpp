#pragma once

#include "gr-base/asset/iasset.hpp"
#include "gr-base/core/geometry/vertex3D.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/asset/default.hpp"

#include <vector>

struct SubMeshAsset
{
    uint32_t indexCount;
    uint32_t indexOffset;
};

struct MeshAsset : IAsset
{
    std::vector<SubMeshAsset> submeshes;

    std::vector<Vertex3D> vertices;

    std::vector<uint32_t> indices;

    std::string name;

    bool serialize(Stream* stream) const override;

    bool deserialize(Stream* stream) override;
};


