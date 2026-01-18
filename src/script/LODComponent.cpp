#include "gr-base/script/LODComponent.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/managers/assetsManager.hpp"

bool LODComponent::Serialize(Stream* stream) const
{
    {
        size_t count = meshesLOD.size();
        if (!stream->write(count))
            return false;
    }

    for (auto &lod : meshesLOD)
    {
        if (!stream->write(lod.minDistance))
            return false;

        size_t count = lod.meshes.size();
        if (!stream->write(count))
            return false;

        for (auto& meshID : lod.meshes)
        {
            auto name = AssetsManager::GetMeshName(meshID);
            if (!stream->write(name))
                return false;
        }
    }
    return true;
}

bool LODComponent::Deserialize(Stream* stream)
{
    size_t lodcount;
    if (!stream->read(lodcount))
        return false;

    for (size_t i=0;i<lodcount;i++)
    {
        auto& lod = meshesLOD.emplace_back();

        if (!stream->read(lod.minDistance))
            return false;

        size_t levelcount;
        if (!stream->read(levelcount))
            return false;

        for (size_t j=0;j<levelcount;j++)
        {
            std::string name;
            if (!stream->read(name))
                return false;

            auto meshID = AssetsManager::FindMesh(name);
            if (meshID != ID_INVALID)
                lod.meshes.push_back(meshID);
        }
    }
    return true;
}


