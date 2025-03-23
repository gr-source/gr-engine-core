#include "gr-base/core/geometry/batchingRender.hpp"

#include "gr-base/core/geometry/boneWeight.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include <gr-render/gVertexArray.h>

BatchingRender::BatchingRender(BatchingRenderFlags flags) : m_vertexBuffer(grr::gVertexArray::Create()), m_vbo(grr::gVertexArray::CreateBuffer(BufferType::VBO)), m_ebo(grr::gVertexArray::CreateBuffer(BufferType::EBO)), m_pbo(grr::gVertexArray::CreateBuffer(BufferType::PBO)), m_indexCount(0), m_flags(flags)
{
}

BatchingRender::~BatchingRender()
{
    if (m_vertexBuffer)
    {
        grr::gVertexArray::Destroy(m_vertexBuffer);
    }

    if (m_vbo)
    {
        grr::gVertexArray::DeleteBuffer(m_vbo);
    }

    if (m_ebo)
    {
        grr::gVertexArray::DeleteBuffer(m_ebo);
    }

    if (m_pbo)
    {
        grr::gVertexArray::DeleteBuffer(m_pbo);
    }
}

void BatchingRender::build(Mesh *mesh)
{
    auto vertices = mesh->getVertices();
    // for (auto vertice : mesh->getVertices()) {
    //     vertices.push_back(VertexData{vertice.position, vertice.normal, vertice.texCoord, vertice.mIdx, entityID});
    // }

    m_indexCount = vertices.size();
    
    m_vertexBuffer->bind();

    /*
    if (m_flags & BatchingFlags_HasIndices) {
        m_EBO = m_EBO != 0 ? m_EBO : grr::gVertexArray::CreateBuffer(BufferType::EBO);

        auto indices = mesh->getIndices();

        m_indexCount = indices.size();

        grr::gVertexArray::Bind(m_EBO);
        grr::gVertexArray::UpdateResizeBuffer(indices.size() * sizeof(grm::u32), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(grm::u32), indices.data());
    }
    */
    
    
    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vertex3D), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex3D), vertices.data());

    grr::gVertexArray::SetAttrib(0, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));
    

    grr::gVertexArray::Bind(m_pbo);
    /*grr::gVertexArray::SetAttribI(3, 1, sizeof(int), nullptr);*/
    /*grr::gVertexArray::SetAttribDivisor(3, 1);*/
    
    auto boneweight = mesh->getBoneWeight();
    if (!boneweight.empty())
    {
        grr::gVertexArray::UpdateResizeBuffer(boneweight.size() * sizeof(BoneWeight), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, boneweight.size() * sizeof(BoneWeight), boneweight.data());

        grr::gVertexArray::SetAttribI(4, 4, sizeof(BoneWeight), reinterpret_cast<const void*>(offsetof(BoneWeight, m_index)));
        /*grr::gVertexArray::SetAttribDivisor(4, 1);*/

        grr::gVertexArray::SetAttrib(5, 4, sizeof(BoneWeight), reinterpret_cast<const void*>(offsetof(BoneWeight, m_weight)));
        /*grr::gVertexArray::SetAttribDivisor(5, 1);*/

    }
    /*
    if (m_flags == BatchingRenderFlags_Default)
    {
        grr::gVertexArray::Bind(m_pbo);
        grr::gVertexArray::SetAttribI(3, 1, sizeof(int), nullptr);
        grr::gVertexArray::SetAttribDivisor(3, 1);
    } else if (m_flags == BatchingRenderFlags_Instanced)
    {
        grr::gVertexArray::Bind(m_pbo);
        grr::gVertexArray::SetAttribI(3, 1, sizeof(InstancedData), reinterpret_cast<const void*>(offsetof(InstancedData, entityID)));
        grr::gVertexArray::SetAttribDivisor(3, 1);

        for (int i=0;i<4;i++)
        {
            grr::gVertexArray::SetAttrib(4 + i, 4, sizeof(InstancedData), reinterpret_cast<const void*>(offsetof(InstancedData, model) + i * sizeof(Vector4)));
            grr::gVertexArray::SetAttribDivisor(4 + i, 1);
        }
    }
    */
    m_vertexBuffer->unbind();
}

void BatchingRender::updatePBO(grm::size stride, const void *data)
{
    m_vertexBuffer->bind();
    grr::gVertexArray::Bind(m_pbo);
    grr::gVertexArray::UpdateResizeBuffer(stride, BufferUsage::DYNAMIC);
    grr::gVertexArray::SetBufferUpdate(0, stride, data);
}

void BatchingRender::render()
{
    m_vertexBuffer->bind();

    switch (m_flags)
    {
    case BatchingRenderFlags_Default:
        return grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, m_indexCount);
    case BatchingRenderFlags_Instanced:
        return grr::gVertexArray::DrawArraysInstanced(PrimitiveType::TRIANGLES, m_indexCount, m_pboCount);
    default:
        break;
    }

    // m_vertexBuffer->unbind();

    // if (BatchingRenderFlags > 0)
    // else
    //     grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, m_indexCount);
}

void BatchingRender::setInstancedCount(grm::size count)
{
    m_pboCount = count;
}
