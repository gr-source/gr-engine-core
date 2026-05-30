#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/file/stream.hpp"

Mesh::Mesh() : m_boundsSphere({Vector3::zero, 0.0f}), m_vertexarray(nullptr), m_pbo(nullptr)
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
    if (m_vertices.empty())
        return;

    Vector3 min = m_vertices[0].position;
    Vector3 max = m_vertices[0].position;

    // Calcula AABB
    for (const auto& vertex : m_vertices)
    {
        min.x = std::min(min.x, vertex.position.x);
        min.y = std::min(min.y, vertex.position.y);
        min.z = std::min(min.z, vertex.position.z);

        max.x = std::max(max.x, vertex.position.x);
        max.y = std::max(max.y, vertex.position.y);
        max.z = std::max(max.z, vertex.position.z);
    }

    m_boundsSphere.center = (min + max) * 0.5f;

    float maxDistSq = 0.0f;

    for (const auto& vertex : m_vertices)
    {
        Vector3 delta = vertex.position - m_boundsSphere.center;

        float distSq =
            delta.x * delta.x +
            delta.y * delta.y +
            delta.z * delta.z;

        maxDistSq = std::max(maxDistSq, distSq);
    }

    m_boundsSphere.radius = std::sqrt(maxDistSq);
}

const BoundingSphere &Mesh::getBounds() const
{
    return m_boundsSphere;
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
        {gr::shader_data_type::Float3, "in_position", false},
        {gr::shader_data_type::Float3, "in_normal", false},
        {gr::shader_data_type::Float2, "in_texCoord", false}
    };

    auto vbo = gr::vertex_buffer::create(m_vertices.data(), m_vertices.size() * sizeof(Vertex3D), gr::buffer_usage::static_draw);
    vbo->SetLayout(layout);

    gr::buffer_layout instanceLayout = {
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow0", true },
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow1", true },
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow2", true },
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow3", true }
    };

    m_pbo = gr::vertex_buffer::create(nullptr, sizeof(Matrix4x4), gr::buffer_usage::dynamic_draw);
    m_pbo->SetLayout(instanceLayout);
    
    std::vector<uint32_t> allIndices;

    uint32_t offset = 0;
    for (auto& submesh : m_submeshes)
    {
        allIndices.insert(allIndices.end(), submesh.indices.begin(), submesh.indices.end());

        submesh.indexCount = static_cast<uint32_t>(submesh.indices.size());
        submesh.indexOffset = offset;

        offset += submesh.indices.size();
    }

    auto ibo = gr::index_buffer::create(allIndices.data(), allIndices.size() * sizeof(uint32_t));

    m_vertexarray = gr::vertex_array::create();
    m_vertexarray->AddVertexBuffer(vbo);
    m_vertexarray->AddVertexBuffer(m_pbo);
    m_vertexarray->SetIndexBuffer(ibo);

    CalculateLocalBounds();
}


