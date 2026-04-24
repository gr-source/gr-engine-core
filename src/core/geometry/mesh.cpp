#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/file/stream.hpp"

Mesh::Mesh() : m_boundbox{Vector3::zero, Vector3::zero}, m_vertexarray(nullptr), m_pbo(nullptr)
{}

// addTriangle
void Mesh::addTriangles(const std::vector<uint32_t> &indices)
{
    m_submeshes.push_back({0, 0, indices});
}

std::vector<BoneWeight> Mesh::getBoneWeight() const
{
    return m_boneWeight;
}

void Mesh::CalculateLocalBounds()
{
    m_boundbox.min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    m_boundbox.max = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
    
    for (auto &vertice : m_vertices)
    {
        m_boundbox.min = Math::min(m_boundbox.min, vertice.position);
        m_boundbox.max = Math::max(m_boundbox.max, vertice.position);
    }
}

const Boundbox &Mesh::getBoundbox() const
{
    return m_boundbox;
}

void Mesh::SetName(const std::string& name)
{
    m_name = name;
}

std::string Mesh::GetName() const
{
    return m_name;
}

void Mesh::Build()
{
    gr::buffer_layout layout = {
        {gr::shader_data_type::Float3, "in_position"},
        {gr::shader_data_type::Float3, "in_normal"},
        {gr::shader_data_type::Float2, "in_texCoord"},
    };

    auto vbo = gr::vertex_buffer::create(m_vertices.data(), m_vertices.size() * sizeof(Vertex3D), gr::buffer_usage::static_draw);
    vbo->SetLayout(layout);

    gr::buffer_layout instanceLayout = {
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow0", true },
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow1", true },
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow2", true },
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow3", true }
    };

    m_pbo = gr::vertex_buffer::create(nullptr, 1024 * sizeof(Matrix4x4), gr::buffer_usage::dynamic_draw);
    m_pbo->SetLayout(instanceLayout);
    
    std::vector<uint32_t> allIndices;

    size_t offset = 0;
    for (auto& submesh : m_submeshes)
    {
        submesh.indexOffset = offset;
        allIndices.insert(allIndices.end(), submesh.indices.begin(), submesh.indices.end());
        submesh.indexCount = submesh.indices.size();
        offset += submesh.indices.size();
    }

    auto ibo = gr::index_buffer::create(allIndices.data(), allIndices.size() * sizeof(uint32_t));

    m_vertexarray = gr::vertex_array::create();
    m_vertexarray->AddVertexBuffer(vbo);
    m_vertexarray->AddVertexBuffer(m_pbo);
    m_vertexarray->SetIndexBuffer(ibo);
}


