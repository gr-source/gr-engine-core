#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/file/stream.hpp"

Mesh::Mesh()
{}

std::vector<Vertex3D> Mesh::getVertices() const {
    return m_vertices;
}

void Mesh::setVertices(const std::vector<Vertex3D> &vertices) {
    m_vertices = vertices;
}

// addTriangle
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

void Mesh::setIndices(const std::vector<int> &indices) {
    m_indices = indices;
}

bool Mesh::hasIndices() const {
    return !m_indices.empty();
}

std::vector<BoneWeight> Mesh::getBoneWeight() const {
    return m_boneWeight;
}

int Mesh::serialize(Stream *stream) const
{
    // Serialize vertices
    auto vSize = m_vertices.size();
    if (!stream->writeS(&vSize, 1)) {
        return 3;
    }

    if (!stream->writeS(m_vertices.data(), vSize)) {
        return 4;
    }

    // Serialize indices
    auto iSize = m_indices.size();
    if (!stream->writeS(&iSize, 1)) {
        return 5;
    }

    if (!stream->writeS(m_indices.data(), iSize)) {
        return 6;
    }

    // Serialize bone
    auto bsize = m_boneWeight.size();
    if (!stream->writeS(&bsize, 1)) {
        return 7;
    }

    if (!stream->writeS(m_boneWeight.data(), m_boneWeight.size())) {
        return 8;
    }
    
    return 0;
}

int Mesh::deserialize(Stream *stream)
{
    // Deserialize vertices
    std::size_t vSize;
    if (!stream->readS(&vSize, 1)) {
        return 3;
    }

    m_vertices.resize(vSize);
    if (!stream->readS(m_vertices.data(), vSize)) {
        return 4;
    }

    // Deserialize indices
    std::size_t iSize;
    if (!stream->readS(&iSize, 1)) {
        return 5;
    }

    m_indices.resize(iSize, 0);
    if (!stream->readS(m_indices.data(), iSize)) {
        return 6;
    }

    std::size_t bsize = 0;
    if (!stream->readS(&bsize, 1)) {
        return 7;
    }

    if (bsize <= 0) { return 0; }

    m_boneWeight.resize(bsize);
    if (!stream->readS(m_boneWeight.data(), m_boneWeight.size())) {
        return 8;
    }
    return 0;
}

