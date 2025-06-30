#include "gr-base/script/baserender.hpp"

#include "gr-base/core/assetsManager.hpp"

#include "gr-base/core/geometry/material.hpp"

#include "gr-base/core/thread/threadpool.hpp"
#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/core/geometry/batching.hpp"

#include <algorithm>
#include <cstddef>

BaseRender::BaseRender() : m_batching(nullptr)
{}

void BaseRender::Init()
{}

void BaseRender::Destroy()
{
    if (m_batching == nullptr)
    {
        return;
    }

    if (Threadpool::IsMainThread())
        m_batching->erase(GetGameObject()->getTransform());

    m_batching = nullptr;
}

bool BaseRender::Serialize(Stream *stream)
{
    auto mcount = m_material.size();
    if (!stream->write(mcount))
    {
        return false;
    }

    for (auto material : m_material)
    {
        auto name = AssetsManager::GetRefMaterialToName(material);
        if (!stream->write(name))
        {
            return false;
        }
    }
    return true;
}

bool BaseRender::Deserialize(Stream *stream)
{
    std::size_t mcount = {};
    if (!stream->read(mcount))
    {
        return false;
    }

    for (std::size_t i=0;i<mcount;i++)
    {
        std::string name;
        if (!stream->read(name))
        {
            return false;
        }

        auto material = AssetsManager::FindMaterial(name);
        if (material == nullptr || hasMaterial(material))
        {
            continue;
        }

        m_material.push_back(material);
    }
    return true;
}

Material *BaseRender::addMaterial(Material *material)
{
    return m_material.emplace_back(material);
}

void BaseRender::delMaterial(Material *material)
{
    m_material.erase(std::remove(m_material.begin(), m_material.end(), material), m_material.end());
}

bool BaseRender::hasMaterial(const Material *material) const
{
    return std::find(m_material.begin(), m_material.end(), material) != m_material.end();
}

std::vector<Material *> BaseRender::getMaterial() const
{
    return m_material;
}

const std::vector<Material *> &BaseRender::getMaterialRef() const
{
    return m_material;
}





