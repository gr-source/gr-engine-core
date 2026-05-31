#pragma once

#include "gr-base/core/geometry/vertex3D.hpp"

#include <gr-render/vertex_array.hpp>

struct Submesh
{
    uint32_t* indices = nullptr;
    size_t numIndices = 0;

    size_t indexCount = 0;
    size_t indexOffset = 0;
};

struct GPUMesh
{
    gr::vertex_array* vertexarray = nullptr;

    gr::vertex_buffer* pbo = nullptr;
};

struct Mesh
{
    Vertex3D* vertices = nullptr;
    size_t numVertices = 0;

    Submesh* submeshes = nullptr;
    size_t numSubmeshes = 0;

    GPUMesh gpuMesh;

    void Build();
};


