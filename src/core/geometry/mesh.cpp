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
    std::size_t vericecount = m_vertices.size();
    if (!stream->write(vericecount))
        return 1;

    if (!stream->write(m_vertices.data(), vericecount * sizeof(Vertex3D)))
        return 2;

    // Serialize indices
    std::size_t indicecount = m_indices.size();
    if (!stream->write(indicecount))
        return 3;

    if (!stream->write(m_indices.data(), indicecount * sizeof(int)))
        return 4;

    // Serialize bone
    std::size_t bonewightcount = m_boneWeight.size();
    if (!stream->write(bonewightcount))
        return 5;

    if (!stream->write(m_boneWeight.data(), bonewightcount * sizeof(BoneWeight)))
        return 6;
    
    return 0;
}

int Mesh::deserialize(Stream *stream)
{
    // Deserialize vertices
    std::size_t vericecount = {};
    if (!stream->read(vericecount))
        return 1;

    m_vertices.resize(vericecount);
    if (!stream->read(reinterpret_cast<char *>(m_vertices.data()), vericecount * sizeof(Vertex3D)))
        return 2;

    // Deserialize indices
    std::size_t indicecount = {};
    if (!stream->read(indicecount))
        return 3;

    m_indices.resize(indicecount, 0);
    if (!stream->read(reinterpret_cast<char *>(m_indices.data()), indicecount * sizeof(int)))
        return 4;

    std::size_t bonewightcount = {};
    if (!stream->read(bonewightcount))
        return 5;

    m_boneWeight.resize(bonewightcount);
    if (!stream->read(reinterpret_cast<char *>(m_boneWeight.data()), bonewightcount * sizeof(BoneWeight)))
        return 6;
    return 0;
}

