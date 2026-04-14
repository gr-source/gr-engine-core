#pragma once


#include "gr-base/core/geometry/vertex3D.hpp"

#include "gr-base/core/file/default.hpp"

#include <vector>

class SubMesh
{
public:
    std::vector<uint32_t> indices;

    uint32_t materialID;
};

class MeshAsset
{
public:
    std::vector<Vertex3D> vertices;

    std::vector<SubMesh> submeshes;

    bool serialize(Stream* stream) const;

    bool deserialize(Stream* stream);
};


