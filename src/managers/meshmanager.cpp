#include "gr-base/managers/meshmanager.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/externals/handle.h"
#include <cassert>
#include <cstdlib>

MeshManager::MeshManager() : m_meshPool(nullptr), m_count(0), m_capacity(0), m_slots(nullptr), m_freeID(0)
{}

MeshHandle MeshManager::create(Hash64 hash)
{
    if (m_count >= m_capacity)
        resize();

    uint32_t id = m_freeID;

    Slot& slot = m_slots[id];
    m_freeID = slot.next;

    m_count++;

    MeshHandle h = handle_create(id, slot.version);

    Mesh* mesh = m_meshPool + id;
    new (mesh) Mesh();

    m_meshMap.emplace(hash, h);

    return h;
}

Mesh *MeshManager::get(MeshHandle h) const
{
    if (h == HANDLE_INVALID)
        return nullptr;

    uint32_t index = handle_get_first(h);
    uint32_t version = handle_get_second(h);

    Slot& slot = m_slots[index];
    if (slot.version != version)
        return nullptr;

    return &m_meshPool[index];
}

void MeshManager::destroy(MeshHandle h)
{
    uint32_t index = handle_get_first(h);
    uint32_t version = handle_get_second(h);

    Slot& slot = m_slots[index];
    if (slot.version != version)
        return;

    m_freeID = index;

    slot.next = m_freeID;
    slot.version++;

    m_count--;
}

MeshHandle MeshManager::find(Hash64 hash) const
{
    auto it = m_meshMap.find(hash);
    if (it == m_meshMap.end())
        return HANDLE_INVALID;
    return it->second;
}

size_t MeshManager::GetCount() const
{
    return m_count;
}

// private
void MeshManager::resize()
{
    size_t oldCapacity = m_capacity;

    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_meshPool = (Mesh*)realloc(m_meshPool, m_capacity * sizeof(Mesh));
    m_slots = (Slot*)realloc(m_slots, m_capacity * sizeof(Slot));

    addFreeID(oldCapacity);
}

void MeshManager::addFreeID(size_t oldCapacity)
{
    size_t lastElement = m_capacity - 1;
    for (size_t i=oldCapacity;i<lastElement;i++)
        m_slots[i] = { static_cast<uint32_t>(i) + 1, 1 };

    m_slots[lastElement] = { m_freeID, 1 };
    m_freeID = static_cast<uint32_t>(oldCapacity);
}



