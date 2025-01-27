#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/file/stream.hpp"

Mesh::Mesh(const std::string &name) : m_name(name) {}

void Mesh::setName(const std::string &name) {
    m_name = name;
}

std::string Mesh::getName() const {
    return m_name;
}

std::vector<Vertex3D> Mesh::getVertices() const {
    return m_vertices;
}

void Mesh::addTriangles(int t0, int t1, int t2) {
    m_indices.push_back(t0);
    m_indices.push_back(t1);
    m_indices.push_back(t2);
}

void Mesh::addIndice(int t0) {
    m_indices.push_back(t0);
}

std::vector<int> Mesh::getIndices() const {
    return m_indices;
}

bool Mesh::hasIndices() const {
    return !m_indices.empty();
}

std::vector<BoneWeight> Mesh::getBoneWeight() const {
    return m_boneWeight;
}

int Mesh::serialize(Stream *streaming) const {
    // Serialzie name
    auto nSize = m_name.size();
    if (!streaming->writeS(&nSize, 1)) {
        return 1;
    }

    if (!streaming->writeS(m_name.data(), nSize)) {
        return 2;
    }

    // Serialize vertices
    auto vSize = m_vertices.size();
    if (!streaming->writeS(&vSize, 1)) {
        return 3;
    }

    if (!streaming->writeS(m_vertices.data(), vSize)) {
        return 4;
    }

    // Serialize indices
    auto iSize = m_indices.size();
    if (!streaming->writeS(&iSize, 1)) {
        return 5;
    }

    if (!streaming->writeS(m_indices.data(), iSize)) {
        return 6;
    }

    // Serialize bone
    auto bsize = m_boneWeight.size();
    if (!streaming->writeS(&bsize, 1)) {
        return 7;
    }

    if (!streaming->writeS(m_boneWeight.data(), m_boneWeight.size())) {
        return 8;
    }
    
    return 0;
}

int Mesh::deserialize(Stream *streaming) {
    // Deserialize name
    std::size_t nSize;
    if (!streaming->readS(&nSize, 1)) {
        return 1;
    }

    m_name.resize(nSize, '\0');
    if (!streaming->readS(m_name.data(), nSize)) {
        return 2;
    }

    // Deserialize vertices
    std::size_t vSize;
    if (!streaming->readS(&vSize, 1)) {
        return 3;
    }

    m_vertices.resize(vSize);
    if (!streaming->readS(m_vertices.data(), vSize)) {
        return 4;
    }

    // Deserialize indices
    std::size_t iSize;
    if (!streaming->readS(&iSize, 1)) {
        return 5;
    }

    m_indices.resize(iSize, 0);
    if (!streaming->readS(m_indices.data(), iSize)) {
        return 6;
    }

    std::size_t bsize = 0;
    if (!streaming->readS(&bsize, 1)) {
        return 7;
    }

    if (bsize <= 0) { return 0; }

    m_boneWeight.resize(bsize);
    if (!streaming->readS(m_boneWeight.data(), m_boneWeight.size())) {
        return 8;
    }
    return 0;
}

