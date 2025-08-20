#include "gr-base/script/meshFilter.hpp"

#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/assetsManager.hpp"

MeshFilter::MeshFilter() : m_mesh(nullptr) {}

void MeshFilter::setMesh(std::vector<Mesh> *mesh)
{
    m_mesh = mesh;

    processBoundbox();
}

std::vector<Mesh> *MeshFilter::getMesh() {
    return m_mesh;
}

void MeshFilter::processBoundbox()
{
    m_boundbox.min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    m_boundbox.max = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
    
    for (const auto& mesh : *m_mesh)
    {
        for (auto &vertice : mesh.getVertices())
        {
            m_boundbox.min = Math::min(m_boundbox.min, vertice.position);
            m_boundbox.max = Math::max(m_boundbox.max, vertice.position);
        }
    }
}

Boundbox &MeshFilter::getBoundbox() {
    return m_boundbox;
}

bool MeshFilter::onSerialize(Stream *stream)
{
    bool hasMesh = m_mesh != nullptr;
    if (!stream->write(&hasMesh, sizeof(bool)))
    {
        return false;
    }

    if (!hasMesh)
    {
        return true;
    }

    if (!stream->write(&m_boundbox, sizeof(Boundbox)))
        return false;

    auto name = AssetsManager::GetMeshListRefToName(m_mesh);
    if (!stream->write(name))
    {
        return false;
    }

    auto mcount = m_mesh->size();
    if (!stream->write(mcount))
    {
        return false;
    }

    for (auto &mesh : *m_mesh)
    {
        mesh.serialize(stream);
    }

    return true;
}

bool MeshFilter::onDeserialize(Stream *stream) {
    bool hasMesh;
    if (!stream->read(reinterpret_cast<char *>(&hasMesh), sizeof(bool)))
    {
        return false;
    }

    if (!hasMesh)
    {
        return true;
    }

    if (!stream->read(reinterpret_cast<char *>(&m_boundbox), sizeof(Boundbox)))
        return false;


    std::string name;
    if (!stream->read(name))
    {
        return false;
    }

    std::size_t mcount = 0;
    if (!stream->read(mcount))
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


