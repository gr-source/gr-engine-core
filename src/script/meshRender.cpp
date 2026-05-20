#include "gr-base/script/meshRender.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/managers/assetsManager.hpp"

void MeshRender::Serialize(Stream *stream, const MeshRender* meshRender)
{
    uint64_t count = static_cast<uint64_t>(meshRender->materials.size());
    if (!stream->write(count))
        return;

    for (auto &materialID : meshRender->materials)
    {
        auto name = AssetsManager::GetMaterialName(materialID);

        if (!stream->write(name))
            return;
    }
    return;
}

#include <exception>
void MeshRender::Deserialize(Stream *stream, MeshRender* meshRender)
{
    uint64_t count = 0;
    if (!stream->read(count))
        return;

    for (uint64_t i=0;i<count;i++)
    {
        std::string name;
        if (!stream->read(name))
            return;

        auto materialID = AssetsManager::FindMaterial(name);
        if (materialID == ID_INVALID)
        {
            try
            {
                materialID = AssetsManager::LoadMaterial(name);
            } catch (const std::exception& e)
            {
                Console::LogError("[BaseRender::Deserialize] %s", e.what());
                continue;
            }
        }

        meshRender->materials.push_back(materialID);
    }

    return;
}


