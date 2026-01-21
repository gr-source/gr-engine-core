#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/file/stream.hpp"

Mesh::Mesh() : m_boundbox{Vector3::zero, Vector3::zero}
{}

std::vector<Vertex3D> Mesh::getVertices() const
{
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

void Mesh::CalculateLocalBounds()
{
    m_boundbox.min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    m_boundbox.max = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
    
    for (auto &vertice : m_vertices)
    {
        m_boundbox.min = Math::min(m_boundbox.min, vertice.position);
        m_boundbox.max = Math::max(m_boundbox.max, vertice.position);
    }
}

const Boundbox &Mesh::getBoundbox() const
{
    return m_boundbox;
}

int Mesh::serialize(Stream *stream) const
{
    if (!stream->write(m_boundbox))
        return 1;

    // Serialize vertices
    std::size_t vericecount = m_vertices.size();
    if (!stream->write(vericecount))
        return 2;

    if (!stream->write(m_vertices.data(), vericecount * sizeof(Vertex3D)))
        return 3;

    // Serialize indices
    std::size_t indicecount = m_indices.size();
    if (!stream->write(indicecount))
        return 4;

    if (!stream->write(m_indices.data(), indicecount * sizeof(int)))
        return 5;

    // Serialize bone
    std::size_t bonewightcount = m_boneWeight.size();
    if (!stream->write(bonewightcount))
        return 6;

    if (!stream->write(m_boneWeight.data(), bonewightcount * sizeof(BoneWeight)))
        return 7;
    
    return 0;
}

int Mesh::deserialize(Stream *stream)
{
    if (!stream->read(m_boundbox))
        return 1;

    // Deserialize vertices
    std::size_t vericecount = {};
    if (!stream->read(vericecount))
        return 2;

    m_vertices.resize(vericecount);
    if (!stream->read(reinterpret_cast<char *>(m_vertices.data()), vericecount * sizeof(Vertex3D)))
        return 3;

    // Deserialize indices
    std::size_t indicecount = {};
    if (!stream->read(indicecount))
        return 4;

    m_indices.resize(indicecount, 0);
    if (!stream->read(reinterpret_cast<char *>(m_indices.data()), indicecount * sizeof(int)))
        return 5;

    std::size_t bonewightcount = {};
    if (!stream->read(bonewightcount))
        return 6;

    m_boneWeight.resize(bonewightcount);
    if (!stream->read(reinterpret_cast<char *>(m_boneWeight.data()), bonewightcount * sizeof(BoneWeight)))
        return 7;
    return 0;
}

void Mesh::SetName(const std::string& name)
{
    m_name = name;
}

std::string Mesh::GetName() const
{
    return m_name;
}


