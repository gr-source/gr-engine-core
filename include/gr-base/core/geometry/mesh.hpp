#pragma once

#include <gr-render/vertex_array.hpp>
#include <vector>

#include "boneWeight.hpp"
#include "vertex3D.hpp"

#include "gr-base/core/file/default.hpp"

struct BoundingSphere
{
    Vector3 center;
    float radius;
};

class SubMesh
{
public:
    uint32_t indexOffset;
    uint32_t indexCount;

    std::vector<uint32_t> indices;
};

class Mesh
{
public:
    Mesh();

    template <typename ...Args>
    void addVertex(Args &&...args);

    inline std::vector<Vertex3D> GetVertices() const
    {
        return m_vertices;
    }

    inline void SetVertices(const std::vector<Vertex3D> &vertices)
    {
        m_vertices = vertices;
    }

    void addTriangles(const std::vector<uint32_t> &indices);

    inline size_t GetSubMeshCount() const
    {
        return m_submeshes.size();
    }

    inline const SubMesh& GetSubMesh(int index) const
    {
        return m_submeshes[index];
    }

    template <typename ...Args>
    BoneWeight &addBoneWeight(Args &&...args);

    std::vector<BoneWeight> getBoneWeight() const;

    void CalculateLocalBounds();

    const BoundingSphere &getBounds() const;

    void SetName(const std::string& name);

    std::string GetName() const;

    inline gr::vertex_array* GetGpuMesh() const
    {
        return m_vertexarray != nullptr ? m_vertexarray.get() : nullptr;
    }

    inline gr::vertex_buffer* GetIntancingBuffer() const
    {
        return m_pbo != nullptr ? m_pbo.get() : nullptr;
    }

    void Build();

private:
    std::vector<BoneWeight> m_boneWeight;

    std::vector<Vertex3D> m_vertices;

    std::vector<SubMesh> m_submeshes;

    BoundingSphere m_boundsSphere;

    std::shared_ptr<gr::vertex_array> m_vertexarray;

    std::shared_ptr<gr::vertex_buffer> m_pbo;

    std::string m_name;
};

template <typename ...Args>
inline void Mesh::addVertex(Args&& ...args)
{
    m_vertices.push_back(Vertex3D{std::forward<Args>(args)...});
}

template <typename... Args>
inline BoneWeight &Mesh::addBoneWeight(Args &&...args) {
    return m_boneWeight.emplace_back(std::forward<Args>(args)...);
}


