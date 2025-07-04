#include "gr-base/script/meshFilter.hpp"

#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/assetsManager.hpp"

MeshFilter::MeshFilter() : m_mesh(nullptr) {}

void MeshFilter::setMesh(std::vector<Mesh> *mesh) {
    m_mesh = mesh;
}

std::vector<Mesh> *MeshFilter::getMesh() {
    return m_mesh;
}

void MeshFilter::processBoundbox() {
    /*
    m_boundbox.min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    m_boundbox.max = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
    
    for (const auto& vertex : m_sharedMesh->getVertices()) {
        m_boundbox.min = Math::min(m_boundbox.min, vertex.position);
        m_boundbox.max = Math::max(m_boundbox.max, vertex.position);
    }
    */
}

Boundbox &MeshFilter::getBoundbox() {
    return m_boundbox;
}

bool MeshFilter::Serialize(Stream *stream) {
    bool hasMesh = m_mesh != nullptr;
    if (!stream->write(&hasMesh, sizeof(bool)))
    {
        return false;
    }

    if (!hasMesh)
    {
        return true;
    }

    auto name = AssetsManager::GetMeshListRefToName(m_mesh);

    auto nsize = name.size();
    if (!stream->write(&nsize, sizeof(std::size_t)))
    {
        return false;
    }

    if (!stream->write(name.c_str(), nsize * sizeof(char)))
    {
        return false;
    }

    auto mcount = m_mesh->size();

    if (!stream->write(&mcount, sizeof(std::size_t)))
    {
        return false;
    }

    for (auto &mesh : *m_mesh)
    {
        mesh.serialize(stream);
    }

    return true;
}

bool MeshFilter::Deserialize(Stream *stream)
{
    bool hasMesh;
    if (!stream->read(reinterpret_cast<char *>(&hasMesh), sizeof(bool)))
    {
        return false;
    }

    if (!hasMesh)
    {
        return true;
    }

    std::size_t nsize = 0;
    if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t)))
    {
        return false;
    }

    std::string name(nsize, '\0');
    if (!stream->read(name.data(), nsize * sizeof(char)))
    {
        return false;
    }

    std::size_t mcount = 0;
    if (!stream->read(reinterpret_cast<char *>(&mcount), sizeof(std::size_t)))
    {
        return false;
    }

    m_mesh = AssetsManager::FindMesh(name);
    if (!m_mesh)
    {
        m_mesh = AssetsManager::CreateMesh(name);
        if (!m_mesh)
        {
            return false;
        }

        for (std::size_t i={};i<mcount;i++)
        {
            m_mesh->emplace_back().deserialize(stream);
        }
    }
    return true;
}


