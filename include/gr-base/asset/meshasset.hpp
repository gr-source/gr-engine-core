#pragma once

#include "gr-base/core/geometry/vertex3D.hpp"

#include <vector>

class MeshAsset
{
public:
    std::vector<Vertex3D> vertices;

    std::vector<uint32_t> indices;
};


