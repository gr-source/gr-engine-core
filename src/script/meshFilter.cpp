#include "gr-base/script/meshFilter.hpp"

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/file/stream.hpp"

#include <iostream>

void MeshFilter::Serialize(Stream* stream, const MeshFilter* meshFilter)
{
    if (meshFilter->meshID == ID_INVALID)
        return;

    auto name = AssetsManager::GetMeshName(meshFilter->meshID);

    if (!stream->write(name))
        return;
    return;
}

void MeshFilter::Deserialize(Stream* stream, MeshFilter* meshFilter)
{
    std::string name;
    if (!stream->read(name))
        return;

    auto meshID = AssetsManager::FindMesh(name);
    if (meshID == ID_INVALID)
        meshID = AssetsManager::LoadMesh(name);

    meshFilter->meshID = meshID;
}

