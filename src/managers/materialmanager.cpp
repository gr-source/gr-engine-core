#include "gr-base/managers/materialmanager.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/material.hpp"
#include <stdexcept>

typedef struct MaterialWrapper
{
    MaterialID next;

    Material material;
} MaterialWrapper;

// public
MaterialManager::MaterialManager() : m_pool(nullptr), m_count(0), m_freeID(ID_INVALID)
{
}

MaterialID MaterialManager::create(const std::string &name, const Material* src)
{
    auto id = allocate();

    MaterialWrapper* wrapper = (MaterialWrapper*)m_pool->vector;

    Material *material = &wrapper[id].material;

    if (src != nullptr)
    {
        new (material) Material(*src);
    } else
    {
        new (material) Material();
    }

    m_materialnamemap[name] = id;
    m_materialmap[id] = name;

    m_count++;
    
    return id;
}

Material *MaterialManager::get(MaterialID id)
{
    MaterialWrapper* wrapper = (MaterialWrapper*)m_pool->vector;

    return &wrapper[id].material;
}

MaterialID MaterialManager::find(const std::string &name)
{
    auto it = m_materialnamemap.find(name);
    if (it == m_materialnamemap.end())
        return ID_INVALID;
    return it->second;
}

const char *MaterialManager::getname(MaterialID id)
{
    auto it = m_materialmap.find(id);
    if (it == m_materialmap.end())
        return nullptr;
    return it->second.c_str();
}

void MaterialManager::destroy(MaterialID id)
{
    throw std::runtime_error("void MaterialManager::remove(MaterialID id)");
}

MaterialID MaterialManager::GetCount() const
{
    return m_count;
}

MaterialID MaterialManager::allocate()
{
    if (m_freeID == ID_INVALID)
        realloc(64);

    MaterialWrapper* wrapper = (MaterialWrapper*)m_pool->vector;

    MaterialID id = m_freeID;
    m_freeID = wrapper[id].next;

    return id;
}

void MaterialManager::realloc(MaterialID size)
{
    size_t lastsize = m_pool != nullptr ? m_pool->length : 0;
    size_t newsize = lastsize + size;

    if (m_pool == nullptr)
        m_pool = array_create(newsize, sizeof(MaterialWrapper));
    else
        m_pool = array_resize(m_pool, newsize, sizeof(MaterialWrapper));

    addFreeID(lastsize);
}

void MaterialManager::addFreeID(MeshID start)
{
    MaterialWrapper* wrapper = (MaterialWrapper*)m_pool->vector;

    MeshID lastID = m_pool->length - 1;
    for (MeshID id=start;id<lastID;id++)
        wrapper[id].next = id + 1;

    wrapper[lastID].next = m_freeID;
    m_freeID = start;
}


