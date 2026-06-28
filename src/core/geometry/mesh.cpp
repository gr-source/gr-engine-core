#include "gr-base/core/geometry/mesh.hpp"
#include <cstring>

struct GPUVec2
{
    float x, y;

    GPUVec2() = default;

    GPUVec2(const Vector2& v) : x(v.x), y(v.y)
    {}

    GPUVec2& operator =(const Vector2& v)
    {
        x = v.x;
        y = v.y;

        return *this;
    }
};


struct GPUVec4
{
    float x, y, z, w;

    GPUVec4(const Vector3& v) : x(v.x), y(v.y), z(v.z), w(0.0f)
    {}

    GPUVec4() = default;

    GPUVec4& operator =(const Vector3& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        w = 0.0f;

        return *this;
    }
};

struct GPUVertex3D
{
    GPUVec4 position;
    GPUVec4 normal;
    GPUVec2 texCoords;
};

void Mesh::Build()
{
    std::vector<GPUVertex3D> gpuVertices(numVertices);
    for (size_t i=0;i<numVertices;i++)
    {
        auto& vertex = vertices[i];

        gpuVertices[i] = GPUVertex3D{ .position = vertex.position, .normal = vertex.normal, .texCoords = vertex.texCoords };
    }

    gr::buffer_layout layout = {
        {gr::shader_data_type::Float4, "in_position", false},
        {gr::shader_data_type::Float4, "in_normal", false},
        {gr::shader_data_type::Float2, "in_texCoord", false}
    };

    auto vbo = gr::vertex_buffer::create(gpuVertices.data(), gpuVertices.size() * sizeof(GPUVertex3D), gr::buffer_usage::static_draw);
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
    gpuMesh.ibo = ibo;
}

void Mesh_Normalize(Mesh* mesh)
{
    if (!mesh->numVertices)
        return;

    for (size_t i=0;i<mesh->numVertices;i++)
        mesh->vertices[i].normal = Vector3::zero;

    auto vertices = mesh->vertices;

    for (size_t i=0;i<mesh->numSubmeshes;i++)
    {
        Submesh& submesh = mesh->submeshes[i];
        if ((submesh.numIndices % 3) == 0)
            continue;

        for (size_t j=0;j<submesh.numIndices;j+=3)
        {
            const int ia = submesh.indices[j + 0];
            const int ib = submesh.indices[j + 1];
            const int ic = submesh.indices[j + 2];

            const Vector3 e1 = vertices[ia].position - vertices[ib].position;
            const Vector3 e2 = vertices[ic].position - vertices[ib].position;
            const Vector3 no = Math::cross( e1, e2 );

            vertices[ia].normal += no;
            vertices[ib].normal += no;
            vertices[ic].normal += no;
        }


        for (size_t j=0;j<submesh.numIndices;j++)
        {
            const int index = submesh.indices[j];

            vertices[index].normal = Math::normalize(vertices[index].normal);
        }
    }
}



