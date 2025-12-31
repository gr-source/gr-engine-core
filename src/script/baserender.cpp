#include "gr-base/script/baserender.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/geometry/material.hpp"

#include <algorithm>

BaseRender::BaseRender()
{}

void BaseRender::onInit()
{}

void BaseRender::onRelease()
{}

void BaseRender::onEnable()
{}

void BaseRender::onDisable()
{
    /*
    if (m_batchID == InvalidBatch)
        return;

    Batching *batching = BatchingManager::Get(m_batchID);
    batching->erase(GetComponent<Transform>());
    */
}

bool BaseRender::onSerialize(Stream *stream)
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

bool BaseRender::onDeserialize(Stream *stream)
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
        if (materialID == nill)
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

const std::vector<MaterialID> &BaseRender::getMaterial() const
{
    return m_materials;
}





