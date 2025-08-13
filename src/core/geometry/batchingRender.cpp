#include "gr-base/core/geometry/batchingRender.hpp"

#include "gr-base/core/geometry/boneWeight.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include <gr-render/gVertexArray.h>

BatchingRender::BatchingRender(BatchingRenderFlags flags) : m_vertexBuffer(new grr::gVertexArray()), m_vbo(grr::gVertexArray::CreateBuffer(BufferType::VBO)), m_ebo(-1), m_pbo(grr::gVertexArray::CreateBuffer(BufferType::VBO)), m_indexCount(0), m_flags(flags)
{
}

BatchingRender::~BatchingRender()
{
    if (m_vertexBuffer)
    {
        delete m_vertexBuffer;
    }

    if (m_vbo)
    {
        grr::gVertexArray::DeleteBuffer(m_vbo);
    }

    if (m_ebo != -1)
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
    auto indices = mesh->getIndices();

    m_indexCount = vertices.size();
    
    m_vertexBuffer->bind();

    if (!indices.empty())
    {
        if (m_ebo == -1)
            m_ebo = grr::gVertexArray::CreateBuffer(BufferType::EBO);

        std::cout << "Draw elements" << std::endl;

        auto indices = mesh->getIndices();

        m_indexCount = indices.size();

        grr::gVertexArray::Bind(m_ebo);
        grr::gVertexArray::UpdateResizeBuffer(indices.size() * sizeof(grm::uint32), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(grm::uint32), indices.data());
    }
    
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
        if (m_ebo != -1)
            return grr::gVertexArray::DrawElements(PrimitiveType::TRIANGLES, m_indexCount, nullptr);
        return grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, m_indexCount);
    case BatchingRenderFlags_Instanced:
        if (m_ebo != -1)
            return grr::gVertexArray::DrawElementsInstanced(PrimitiveType::TRIANGLES, m_indexCount, nullptr, m_pboCount);
        return grr::gVertexArray::DrawArraysInstanced(PrimitiveType::TRIANGLES, m_indexCount, m_pboCount);
    default:
        break;
    }
}

void BatchingRender::setInstancedCount(grm::size count)
{
    m_pboCount = count;
}
