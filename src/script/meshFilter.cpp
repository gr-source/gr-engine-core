#include "script/meshFilter.hpp"

#include "core/geometry/mesh.hpp"

#include "core/file/stream.hpp"

#include "core/assetsManager.hpp"

MeshFilter::MeshFilter() : m_sharedMesh(nullptr) {}

void MeshFilter::setMesh(Mesh *mesh) {
    m_sharedMesh = mesh;
}

Mesh *MeshFilter::getMesh() {
    return m_sharedMesh;
}

void MeshFilter::processBoundbox() {
    m_boundbox.min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    m_boundbox.max = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
    
    for (const auto& vertex : m_sharedMesh->getVertices()) {
        m_boundbox.min = Math::min(m_boundbox.min, vertex.position);
        m_boundbox.max = Math::max(m_boundbox.max, vertex.position);
    }
}

Boundbox &MeshFilter::getBoundbox() {
    return m_boundbox;
}

bool MeshFilter::Serialize(Stream *stream) {
    bool hasMesh = m_sharedMesh != nullptr;
    if (!stream->write(reinterpret_cast<const void *>(&hasMesh), sizeof(bool))) {
        return false;
    }

    if (!hasMesh) {
        return true;
    }
    auto name = m_sharedMesh->getName();

    auto nSize = name.size();
    if (!stream->write(reinterpret_cast<const char *>(&nSize), sizeof(std::size_t))) {
        return false;
    }

    if (!stream->write(name.data(), nSize)) {
        return false;
    }

    m_sharedMesh->serialize(stream);

    return true;
}

bool MeshFilter::Deserialize(Stream *stream) {
    bool hasMesh;
    if (!stream->read(reinterpret_cast<char *>(&hasMesh), sizeof(bool))) {
        return false;
    }

    if (!hasMesh) {
        return true;
    }

    std::size_t nsize = 0;
    if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t))) {
        return false;
    }

    std::string name(nsize, '\0');
    if (!stream->read(name.data(), nsize)) {
        return false;
    }

    m_sharedMesh = AssetsManager::FindMesh(name);
    if (!m_sharedMesh) {
        m_sharedMesh = AssetsManager::CreateMesh(name, name);
        if (!m_sharedMesh) {
            return false;
        }
        m_sharedMesh->deserialize(stream);
    }
    return true;
}


