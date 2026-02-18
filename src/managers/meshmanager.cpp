#include "gr-base/managers/meshmanager.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/externals/array.h"

typedef struct MeshWrapper
{
    MeshID next;

    Mesh mesh;
} MeshWrapper;

MeshManager::MeshManager() : m_pool(nullptr), m_count(0), m_freeID(ID_INVALID)
{}

MeshID MeshManager::create(const std::string &name, const Mesh* src)
{
    MeshID id = allocate();

    MeshWrapper* wrapper = (MeshWrapper*)m_pool->vector;

    if (src != nullptr)
    {
        new (&wrapper[id].mesh) Mesh(*src);
    } else
    {
        new (&wrapper[id].mesh) Mesh();
    }

    m_meshnamemap[id] = name;
    m_meshidmap[name] = id;

    return id;
}

Mesh *MeshManager::get(MeshID id)
{
    MeshWrapper* wrapper = (MeshWrapper*)m_pool->vector;

    return &wrapper[id].mesh;
}

void MeshManager::destroy(MeshID id)
{
    Mesh* mesh = get(id);
    if (mesh == nullptr)
        return;

    mesh->~Mesh();

    m_meshidmap.erase(m_meshnamemap[id]);
    m_meshnamemap.erase(id);
}

MeshID MeshManager::allocate()
{
    if (m_freeID == ID_INVALID)
        realloc(64);

    MeshWrapper* wrapper = (MeshWrapper*)m_pool->vector;

    MeshID id = m_freeID;
    m_freeID = wrapper[id].next;

    m_count++;

    return id;
}

void MeshManager::deallocate(MeshID id)
{
    MeshWrapper* wrapper = (MeshWrapper*)m_pool->vector;
    wrapper[id].next = m_freeID;
    m_freeID = id;

    m_count--;
}

MeshID MeshManager::find(const std::string &name)
{
    auto it = m_meshidmap.find(name);
    if (it == m_meshidmap.end())
        return ID_INVALID;
    return it->second;
}

const char *MeshManager::getname(MeshID id)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_meshnamemap.find(id);
    if (it == m_meshnamemap.end())
        return nullptr;
    return it->second.c_str();
}

size_t MeshManager::getCount() const
{
    return m_count;
}

// private
void MeshManager::realloc(size_t count)
{
    size_t lastsize = m_pool != nullptr ? m_pool->length : 0;
    size_t newsize = lastsize + count;

    if (m_pool == nullptr)
        m_pool = array_create(newsize, sizeof(MeshWrapper));
    else
        m_pool = array_resize(m_pool, newsize, sizeof(MeshWrapper));

    addFreeID(lastsize);
}

void MeshManager::addFreeID(MeshID start)
{
    MeshWrapper* wrapper = (MeshWrapper*)m_pool->vector;

    MeshID lastID = m_pool->length - 1;
    for (MeshID id=start;id<lastID;id++)
        wrapper[id].next = id + 1;

    wrapper[lastID].next = m_freeID;
    m_freeID = start;
}


