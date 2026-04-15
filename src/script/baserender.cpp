#include "gr-base/script/baserender.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include <algorithm>

BaseRender::BaseRender()
{}

bool BaseRender::Serialize(Stream *stream) const
{
    auto count = m_materials.size();
    if (!stream->write(count))
        return false;

    for (auto &materialID : m_materials)
    {
        auto name = AssetsManager::GetMaterialName(materialID);

        if (!stream->write(name))
            return false;
    }
    return true;
}

bool BaseRender::Deserialize(Stream *stream)
{
    std::size_t count = {};
    if (!stream->read(count))
        return false;

    for (std::size_t i=0;i<count;i++)
    {
        std::string name;
        if (!stream->read(name))
            return false;

        auto materialID = AssetsManager::FindMaterial(name);
        if (materialID == ID_INVALID)
            continue;

        m_materials.push_back(materialID);
    }
    return true;
}





