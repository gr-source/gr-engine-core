#include "gr-base/core/task/materialmanager.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/material.hpp"

// public
MaterialManager::MaterialManager()
{
    resize(1024);
}

MaterialID MaterialManager::create(grr::gShader *shader, const std::string &name)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_freeIDs.empty())
    {
        lock.unlock();

        resize(1024);

        lock.lock();
    }

    auto id = m_freeIDs.top();
    m_freeIDs.pop();

    Material *material = &m_materials[id];
    new (material) Material(shader);

    m_materialnamemap[name] = id;
    m_materialmap[id] = name;
    
    return id;
}

Material &MaterialManager::get(MaterialID id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_materials[id];
}

MaterialID MaterialManager::find(const std::string &name)
{
    auto it = m_materialnamemap.find(name);
    if (it == m_materialnamemap.end())
        return nill;
    return it->second;
}

const char *MaterialManager::getname(MaterialID id)
{
    auto it = m_materialmap.find(id);
    if (it == m_materialmap.end())
        return nullptr;
    return it->second.c_str();
}

void MaterialManager::remove(MaterialID id)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_materials[id].~Material();

    auto it = m_materialmap.find(id);
    if (it != m_materialmap.end())
    {
        m_materialnamemap.erase(it->second);

        m_materialmap.erase(it);
    }
}

void MaterialManager::resize(std::size_t size)
{
    auto newsize = m_materials.size() + size;
    auto oldsize = m_materials.size();

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_materials.resize(newsize, Material{nullptr});
    }

    addfreelist(oldsize);
}

void MaterialManager::addfreelist(MaterialID id)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto size = m_materials.size() - 1;
    while (id < (size--))
    {
        m_freeIDs.push(size );
    }
}


