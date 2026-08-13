#pragma once

#include "gr-base/core/geometry/vertex3D.hpp"

#include <gr-math/aabb.hpp>
#include <gr-render/vertex_array.hpp>

typedef struct GPUMesh
{
    gr::vertex_array* vertexarray = nullptr;

    gr::vertex_buffer* pbo = nullptr;

    gr::vertex_buffer* vbo = nullptr;

    gr::index_buffer* ibo = nullptr;
} GPUMesh;

typedef struct Submesh
{
    size_t indexCount = 0;
    size_t indexOffset = 0;
} Submesh;

struct Mesh
{
    Vertex3D* vertices = nullptr;
    size_t numVertices = 0;

    uint32_t* indices = nullptr;
    size_t numIndices = 0;

    Submesh* submeshes = nullptr;
    size_t numSubmeshes = 0;

    AABB localBounds =
        {
            type_traits<Vector3>::zero,
            type_traits<Vector3>::zero
        };

    GPUMesh gpuMesh;

    void Build();
};

void Mesh_Normalize(Mesh* mesh);

AABB Mesh_CalculateBounds(Mesh* mesh);


