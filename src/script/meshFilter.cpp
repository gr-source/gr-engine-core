#include "gr-base/script/meshFilter.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/managers/assetsManager.hpp"

MeshFilter::MeshFilter() : m_meshID(ID_INVALID)
{}

bool MeshFilter::Serialize(Stream *stream) const
{
    if (m_meshID == ID_INVALID)
        return false;

    auto name = AssetsManager::GetMeshName(m_meshID);

    if (!stream->write(name))
        return false;
    return true;
}

bool MeshFilter::Deserialize(Stream *stream)
{
    std::string name;
    if (!stream->read(name))
        return false;

    auto meshID = AssetsManager::FindMesh(name);
    if (meshID == ID_INVALID)
        meshID = AssetsManager::LoadMesh(name);

    SetMesh(meshID);
    return true;
}


