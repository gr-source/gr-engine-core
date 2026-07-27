#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

#include <gr-render/default.hpp>

#include "gr-base/pool/objectPool.hpp"

#include <unordered_map>

class TextureManager
{
public:
    TextureManager();
    ~TextureManager();

    TextureHandle create(AssetHash hashCache = INVALID_HASH64);

    gr::Texture *get(TextureHandle h) const;

    void remove(TextureHandle h);

    TextureHandle find(AssetHash hash) const;

    size_t GetCount() const;

private:
    std::unordered_map<AssetHash, TextureHandle> m_hashmap;

    ObjectPool<gr::Texture> m_pool;
};



