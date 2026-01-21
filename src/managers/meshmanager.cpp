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

MeshID MeshManager::allocate(const std::string &name)
{
    MeshID id = ID_INVALID;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_freeID == ID_INVALID)
            realloc(256);

        MeshWrapper* wrapper = (MeshWrapper*)m_pool->vector;

        id = m_freeID;
        m_freeID = wrapper[id].next;

        new (&wrapper[id].mesh) Mesh();

        m_meshnamemap[id] = name;
        m_meshidmap[name] = id;
    }

    m_count++;

    auto scene = Scene::GetCurrent();
    if (scene != nullptr)
        scene->AddMesh(id);

    return id;
}

void MeshManager::deallocate(MeshID id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    MeshWrapper* wrapper = (MeshWrapper*)m_pool->vector;
    wrapper[id].mesh.~Mesh();

    wrapper[id].next = m_freeID;
    m_freeID = id;

    m_meshidmap.erase(m_meshnamemap[id]);
    m_meshnamemap.erase(id);

    m_count--;
}

Mesh *MeshManager::get(MeshID id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    MeshWrapper* wrapper = (MeshWrapper*)m_pool->vector;

    return &wrapper[id].mesh;
}

MeshID MeshManager::find(const std::string &name)
{
    std::lock_guard<std::mutex> lock(m_mutex);

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

void MeshManager::addFreeID(MeshID first)
{
    MeshWrapper* wrapper = (MeshWrapper*)m_pool->vector;

    MeshID lastID = m_pool->length - 1;
    for (MeshID id=first;id<lastID;id++)
        wrapper[id].next = id + 1;

    wrapper[lastID].next = m_freeID;
    m_freeID = first;
}


