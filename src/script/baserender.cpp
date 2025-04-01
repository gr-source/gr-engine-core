#include "gr-base/script/baserender.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/geometry/material.hpp"

#include "gr-base/ecs/gameObject.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/skinnedMeshRender.hpp"

#include <algorithm>
#include <cstddef>

BaseRender::BaseRender()
{}

void BaseRender::Init()
{}

void BaseRender::Destroy()
{}

bool BaseRender::Serialize(Stream *stream)
{
    auto mcount = m_material.size();
    if (!stream->write(&mcount, sizeof(std::size_t)))
    {
        return false;
    }

    for (auto material : m_material)
    {
        auto name = AssetsManager::GetRefMaterialToName(material);

        auto ncount = name.size();
        if (!stream->write(&ncount, sizeof(std::size_t)))
        {
            return false;
        }

        if (!stream->write(name.c_str(), ncount * sizeof(char)))
        {
            return false;
        }
    }
    return true;
}

bool BaseRender::Deserialize(Stream *stream)
{
    std::size_t mcount = 0;
    if (!stream->read(reinterpret_cast<char *>(&mcount), sizeof(std::size_t)))
    {
        return false;
    }

    for (std::size_t i=0;i<mcount;i++)
    {
        std::size_t ncount = 0;
        if (!stream->read(reinterpret_cast<char *>(&ncount), sizeof(std::size_t)))
        {
            return false;
        }

        std::string name(ncount, '\0');
        if (!stream->read(name.data(), ncount * sizeof(char)))
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





