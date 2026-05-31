#include "gr-base/core/geometry/mesh.hpp"
#include <cstring>

void Mesh::Build()
{
    gr::buffer_layout layout = {
        {gr::shader_data_type::Float4, "in_position", false},
        {gr::shader_data_type::Float4, "in_normal", false},
        {gr::shader_data_type::Float2, "in_texCoord", false}
    };

    auto vbo = gr::vertex_buffer::create(vertices, numVertices * sizeof(Vertex3D), gr::buffer_usage::static_draw);
    vbo->SetLayout(layout);

    gr::buffer_layout instanceLayout = {
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow0", true },
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow1", true },
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow2", true },
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow3", true }
    };

    auto pbo = gr::vertex_buffer::create(nullptr, sizeof(Matrix4x4), gr::buffer_usage::dynamic_draw);
    pbo->SetLayout(instanceLayout);
    
    std::vector<uint32_t> allIndices;

    uint32_t offset = 0;
    for (size_t i=0;i<numSubmeshes;i++)
    {
        auto& submesh = submeshes[i];

        std::vector<uint32_t> indices(submesh.numIndices, 0);
        mempcpy(indices.data(), submesh.indices, submesh.numIndices * sizeof(uint32_t));

        allIndices.insert(allIndices.end(), indices.begin(), indices.end());

        submesh.indexCount = static_cast<uint32_t>(indices.size());
        submesh.indexOffset = offset;

        offset += indices.size();
    }

    auto ibo = gr::index_buffer::create(allIndices.data(), allIndices.size() * sizeof(uint32_t));

    auto vertexarray = gr::vertex_array::create();
    vertexarray->AddVertexBuffer(vbo);
    vertexarray->AddVertexBuffer(pbo);
    vertexarray->SetIndexBuffer(ibo);

    gpuMesh.vertexarray = vertexarray;
    gpuMesh.pbo = pbo;
}


