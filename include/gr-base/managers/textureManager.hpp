#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

#include <gr-render/default.hpp>

#include <unordered_map>

class TextureManager
{
public:
    TextureManager();

    TextureHandle create(AssetHash hash);

    gr::Texture *get(TextureHandle h) const;

    void remove(TextureHandle h);

    TextureHandle find(AssetHash hash) const;

    size_t GetCount() const;

private:
    struct Slot
    {
        uint32_t next;
        uint32_t version;
        AssetHash hash;
    };

    std::unordered_map<AssetHash, TextureHandle> m_hashmap;

    gr::Texture* m_pool;

    size_t m_capacity;

    size_t m_count;

    Slot *m_slots;
    uint32_t m_freeID;

    void resize();

    void addFreeID(size_t oldCapacity);
};



