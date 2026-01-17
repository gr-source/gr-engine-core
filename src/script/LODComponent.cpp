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

    for (auto &meshLOD : meshesLOD)
    {
        size_t count = meshLOD.levels.size();
        if (!stream->write(count))
            return false;

        for (auto& level : meshLOD.levels)
        {
            auto name = AssetsManager::GetMeshName(level.meshID);
            if (!stream->write(name))
                return false;

            if (!stream->write(level.minDistance))
                return false;

        }
    }
    return true;
}

bool LODComponent::Deserialize(Stream* stream)
{
    size_t meshescout;
    if (!stream->read(meshescout))
        return false;

    for (size_t i=0;i<meshescout;i++)
    {
        auto& meshLOD = meshesLOD.emplace_back();

        size_t levelcount;
        if (!stream->read(levelcount))
            return false;

        for (size_t j=0;j<levelcount;j++)
        {
            std::string name;
            if (!stream->read(name))
                return false;

            float distance;
            if (!stream->read(distance))
                return false;

            auto meshID = AssetsManager::FindMesh(name);
            if (meshID != nill)
                meshLOD.levels.push_back(LODLevel{meshID, distance});

            // meshLOD.levels.push_back("");
        }
    }
    return true;
}


