#include "gr-base/core/geometry/gpumesh.hpp"

#include <gr-render/gVertexArray.h>

// MeshInfo
void MeshInfo::push(const Matrix4x4 *data, const std::size_t &size)
{
    vertexArray->bind();
    grr::gVertexArray::Bind(pbo);
    grr::gVertexArray::UpdateResizeBuffer(size * sizeof(Matrix4x4), BufferUsage::DYNAMIC);
    grr::gVertexArray::SetBufferUpdate(0, size * sizeof(Matrix4x4), data);
}

void MeshInfo::draw(const std::size_t &instances)
{
    vertexArray->bind();
    if (instances > 0)
    {
        if (ebo != -1)
            return grr::gVertexArray::DrawElementsInstanced(PrimitiveType::TRIANGLES, count, nullptr, instances);
        return grr::gVertexArray::DrawArraysInstanced(PrimitiveType::TRIANGLES, count, instances);
    }

    if (ebo != -1)
        return grr::gVertexArray::DrawElements(PrimitiveType::TRIANGLES, count, nullptr);
    grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, count);
}

// gpuMesh
void GpuMesh::resize(const std::size_t &size)
{
    meshinfolist.resize(size);
    shaderlist.resize(size, nullptr);
}

MeshInfo &GpuMesh::get(const std::size_t &id)
{
    return meshinfolist[id];
}

void GpuMesh::push(const Matrix4x4 *data, const std::size_t &size)
{
    for (auto &meshinfo : meshinfolist)
        meshinfo.push(data, size);
}

void GpuMesh::draw(int id, const std::size_t &instances)
{
    meshinfolist[id].draw(instances);
}
