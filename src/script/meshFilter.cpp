#include "gr-base/script/meshFilter.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/assetsManager.hpp"

MeshFilter::MeshFilter()
{}

void MeshFilter::addMesh(MeshID id)
{
    m_meshes.push_back(id);
}

const std::vector<MeshID> &MeshFilter::getMesh() const
{
    return m_meshes;
}

void MeshFilter::processBoundbox()
{
    /*
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
    */
}

Boundbox &MeshFilter::getBoundbox()
{
    return m_boundbox;
}

bool MeshFilter::onSerialize(Stream *stream)
{
    if (!stream->write(m_boundbox))
        return false;

    auto count = m_meshes.size();
    if (!stream->write(count))
        return false;

    for (auto &meshID : m_meshes)
    {
        auto name = AssetsManager::GetMeshName(meshID);

        if (!stream->write(name))
            return false;
    }
    return true;
}

bool MeshFilter::onDeserialize(Stream *stream)
{
    if (!stream->read(m_boundbox))
        return false;

    std::size_t count = {};
    if (!stream->read(count))
        return false;

    for (std::size_t i=0;i<count;i++)
    {
        std::string name;
        if (!stream->read(name))
            return false;

        auto meshID = AssetsManager::FindMesh(name);
        if (meshID != nill)
        {
            addMesh(meshID);
        } else
        {
            Console::LogError("[ MeshFilter ] mesh %s do not found!", name.c_str());
        }
    }
    return true;
}


