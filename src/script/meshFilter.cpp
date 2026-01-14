#include "gr-base/script/meshFilter.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/managers/assetsManager.hpp"

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

bool MeshFilter::Serialize(Stream *stream) const
{
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

bool MeshFilter::Deserialize(Stream *stream)
{
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


