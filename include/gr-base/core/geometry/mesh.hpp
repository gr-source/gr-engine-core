#pragma once

#include <vector>

#include "boneWeight.hpp"
#include "vertex3D.hpp"

#include "gr-base/core/file/default.hpp"

class Mesh {
public:
    Mesh(const std::string &name = "default");

    void setName(const std::string &name);

    std::string getName() const;

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

    int serialize(Stream *streaming) const;

    int deserialize(Stream *streaming);

private:
    std::vector<BoneWeight> m_boneWeight;

    std::vector<Vertex3D> m_vertices;

    std::vector<int> m_indices;

    std::string m_name;
};

template <typename ...Args>
inline std::size_t Mesh::addVertex(Args&& ...args)
{
    m_vertices.emplace_back(std::forward<Args>(args)...);

    return m_vertices.size() - 1;
}

template <typename... Args>
inline BoneWeight &Mesh::addBoneWeight(Args &&...args) {
    return m_boneWeight.emplace_back(std::forward<Args>(args)...);
}


