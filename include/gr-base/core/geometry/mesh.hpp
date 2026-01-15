#pragma once

#include <vector>

#include "boneWeight.hpp"
#include "gr-base/core/geometry/boundbox.hpp"
#include "vertex3D.hpp"

#include "gr-base/core/file/default.hpp"

class Mesh {
public:
    Mesh();

    template <typename ...Args>
    std::size_t addVertex(Args &&...args);

    std::vector<Vertex3D> getVertices() const;

    void setVertices(const std::vector<Vertex3D> &vertices);

    void addTriangles(int t0, int t1, int t2);

    void addIndice(int t0);

    std::vector<int> getIndices() const;

    void setIndices(const std::vector<int> &indices);

    bool hasIndices() const;

    template <typename ...Args>
    BoneWeight &addBoneWeight(Args &&...args);

    std::vector<BoneWeight> getBoneWeight() const;

    void CalculateLocalBounds();

    const Boundbox &getBoundbox() const;

    int serialize(Stream *stream) const;

    int deserialize(Stream *stream);

private:
    std::vector<BoneWeight> m_boneWeight;

    std::vector<Vertex3D> m_vertices;

    std::vector<int> m_indices;

    Boundbox m_boundbox;
};

template <typename ...Args>
inline std::size_t Mesh::addVertex(Args&& ...args)
{
    m_vertices.push_back(Vertex3D{std::forward<Args>(args)...});

    return m_vertices.size() - 1;
}

template <typename... Args>
inline BoneWeight &Mesh::addBoneWeight(Args &&...args) {
    return m_boneWeight.emplace_back(std::forward<Args>(args)...);
}


