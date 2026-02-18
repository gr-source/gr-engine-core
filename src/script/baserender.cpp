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
        {
            return false;
        }

        auto materialID = AssetsManager::FindMaterial(name);
        if (materialID == ID_INVALID)
            continue;

        addMaterial(materialID);
    }
    return true;
}

void BaseRender::addMaterial(MaterialID id)
{
    m_materials.push_back(id);
}

bool BaseRender::hasMaterial(MaterialID id) const
{
    return std::find(m_materials.begin(), m_materials.end(), id) != m_materials.end();
}

std::vector<MaterialID> BaseRender::getMaterialIDs() const
{
    return m_materials;
}

void BaseRender::removeMaterial(MaterialID id)
{
    m_materials.erase(std::remove(m_materials.begin(), m_materials.end(), id), m_materials.end());
}





