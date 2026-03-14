#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/status.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include <cassert>
#include <gr-render/gl.h>
#include <gr-render/gVertexArray.h>
#include <gr-render/index_buffer.hpp>

Batching::Batching() : m_vertexarray(nullptr)
{}

Batching::~Batching()
{
    // glDeleteVertexArrays(1, &vao);
    // glDeleteBuffers(1, &vbo);
    // glDeleteBuffers(1, &ebo);
    // glDeleteBuffers(1, &pbo);
}

void Batching::insert(const Matrix4x4& matrix)
{
    instances.push_back(matrix);
}

void Batching::build(MeshID meshID, MaterialID materialID)
{
    Mesh* mesh = AssetsManager::GetMesh(meshID);

    auto vertices = mesh->getVertices();
    auto indices = mesh->getIndices();

    gr::buffer_layout layout = {
        {gr::shader_data_type::Float3, "in_position"},
        {gr::shader_data_type::Float3, "in_normal"},
        {gr::shader_data_type::Float2, "in_texCoord"},
    };

    auto vbo = gr::vertex_buffer::create(vertices.data(), vertices.size() * sizeof(Vertex3D), gr::buffer_usage::static_draw);
    vbo->SetLayout(layout);

    gr::buffer_layout instanceLayout = {
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow0", true },
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow1", true },
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow2", true },
        { gr::shader_data_type::Float4, "a_InstanceMatrixRow3", true }
    };

    m_pbo = gr::vertex_buffer::create(nullptr, 1024 * sizeof(Matrix4x4), gr::buffer_usage::dynamic_draw);
    m_pbo->SetLayout(instanceLayout);

    auto ibo = gr::index_buffer::create(indices.data(), indices.size() * sizeof(uint32_t));

    m_vertexarray = gr::vertex_array::create();
    m_vertexarray->AddVertexBuffer(vbo);
    m_vertexarray->AddVertexBuffer(m_pbo);
    m_vertexarray->SetIndexBuffer(ibo);

    m_indexCount = indices.size();

    m_materialID = materialID;
}

void Batching::render()
{
    bool isInstancing = instances.size() > 1;

    auto material = AssetsManager::GetMaterial(m_materialID);
    auto shader = material->GetShader(isInstancing);

    bool enable[10] = {false};
    int textureID[10] = {0};

    shader->bind();
    material->flush(shader);

    shader->setUniform("u_texture", textureID);
    shader->setUniform("u_enable", enable);

    m_vertexarray->Bind();
    if (isInstancing)
    {
        m_pbo->Bind();
        m_pbo->SetData(instances.data(), instances.size() * sizeof(Matrix4x4));
        m_pbo->Unbind();

        glDrawElementsInstanced(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr, instances.size());
    } else if (!instances.empty())
    {
        shader->setUniform("u_model", &instances[0]);

        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    }
    shader->unbind();
    m_vertexarray->Unbind();

    Status::BatchDrawCallCount++;
    Status::BatchRenderCount += instances.size();

    instances.clear();
}






