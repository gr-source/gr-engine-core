#include "gr-base/managers/textureManager.hpp"

#include "gr-base/externals/handle.h"

#include <gr-render/gTexture.h>

#include <cassert>
#include <cstdlib>

TextureManager::TextureManager() : m_pool(nullptr), m_count(0), m_capacity(0), m_slots(nullptr), m_freeID(0)
{}

TextureHandle TextureManager::create(AssetHash hash)
{
    if (m_count >= m_capacity)
        resize();

    uint32_t id = m_freeID;

    Slot& slot = m_slots[id];
    slot.hash = hash;

    m_freeID = slot.next;

    m_count++;

    TextureHandle h = handle_create(id, slot.version);

    gr::Texture* tx = m_pool + id;
    new (tx) gr::Texture();

    if (hash != INVALID_HASH64)
        m_hashmap.emplace(hash, h);

    return h;
}

gr::Texture *TextureManager::get(TextureHandle h) const
{
    uint32_t index = handle_get_first(h);
    uint32_t version = handle_get_second(h);

    Slot& slot = m_slots[index];
    if (slot.version != version)
        return nullptr;

    return &m_pool[index];
}

void TextureManager::remove(TextureHandle h)
{
    uint32_t index = handle_get_first(h);
    uint32_t version = handle_get_second(h);

    Slot& slot = m_slots[index];
    if (slot.version != version)
        return;

    gr::Texture& mesh = m_pool[index];
    mesh.~Texture();

    if (slot.hash != INVALID_HASH64)
        m_hashmap.erase(slot.hash);

    slot.next = m_freeID;
    m_freeID = index;

    slot.version++;

    m_count--;
}

TextureHandle TextureManager::find(AssetHash hash) const
{
    auto it = m_hashmap.find(hash);
    if (it == m_hashmap.end())
        return HANDLE_INVALID;
    return it->second;
}

size_t TextureManager::GetCount() const
{
    return m_count;
}

// private
void TextureManager::resize()
{
    size_t oldCapacity = m_capacity;

    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_pool = (gr::Texture*)realloc(m_pool, m_capacity * sizeof(gr::Texture));
    m_slots = (Slot*)realloc(m_slots, m_capacity * sizeof(Slot));

    addFreeID(oldCapacity);
}

void TextureManager::addFreeID(size_t oldCapacity)
{
    size_t lastElement = m_capacity - 1;
    for (size_t i=oldCapacity;i<lastElement;i++)
        m_slots[i] = { static_cast<uint32_t>(i) + 1, 1, INVALID_HASH64 };

    m_slots[lastElement] = { m_freeID, 1, INVALID_HASH64 };
    m_freeID = static_cast<uint32_t>(oldCapacity);
}



