#include "gr-base/managers/meshmanager.hpp"

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/externals/handle.h"

#include <cassert>
#include <cstdlib>

MeshManager::MeshManager() :
    m_pool(nullptr),
    m_count(0),
    m_capacity(0),
    m_slots(nullptr),
    m_freeID(INVALID_MESHID)
{
}

MeshHandle MeshManager::create()
{
    if (m_freeID == INVALID_MESHID)
        resize();

    uint32_t id = m_freeID;

    Slot& slot = m_slots[id];

    m_freeID = slot.next;

    m_count++;

    MeshHandle h = handle_create(id, slot.version);

    Mesh* mesh = m_pool + id;
    new (mesh) Mesh();

    return h;
}

Mesh *MeshManager::get(MeshHandle h) const
{
    uint32_t index = handle_get_first(h);
    uint32_t version = handle_get_second(h);

    if (index >= m_capacity)
        return nullptr;

    Slot& slot = m_slots[index];
    if (slot.version != version)
        return nullptr;

    return &m_pool[index];
}

void MeshManager::remove(MeshHandle h)
{
    uint32_t index = handle_get_first(h);
    uint32_t version = handle_get_second(h);

    if (index >= m_capacity)
        return;

    Slot& slot = m_slots[index];
    if (slot.version != version)
        return;

    Mesh& mesh = m_pool[index];
    mesh.~Mesh();

    slot.next = m_freeID;
    m_freeID = index;

    slot.version++;

    m_count--;
}

size_t MeshManager::GetCount() const
{
    return m_count;
}

void MeshManager::addInCache(AssetID assetID, MeshHandle handle)
{
    uint32_t index = handle_get_first(handle);
    if (index >= meshIDCache.size())
        meshIDCache.resize(index + 1, INVALID_ASSET);

    meshIDCache[index] = assetID;
}

AssetID MeshManager::findInCache(MeshHandle handle)
{
    uint32_t index = handle_get_first(handle);
    if (index >= meshIDCache.size())
        return INVALID_ASSET;

    return meshIDCache[index];
}

// private
void MeshManager::resize()
{
    size_t oldCapacity = m_capacity;

    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_pool = (Mesh*)realloc(m_pool, m_capacity * sizeof(Mesh));
    m_slots = (Slot*)realloc(m_slots, m_capacity * sizeof(Slot));

    addFreeID(oldCapacity);
}

void MeshManager::addFreeID(size_t oldCapacity)
{
    size_t lastElement = m_capacity - 1;
    for (size_t i=oldCapacity;i<lastElement;i++)
        m_slots[i] = { static_cast<MeshID>(i) + 1, 1 };

    m_slots[lastElement] = { m_freeID, 1 };
    m_freeID = static_cast<uint32_t>(oldCapacity);
}



