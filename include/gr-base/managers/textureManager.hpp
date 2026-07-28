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

    TextureHandle create();

    gr::texture *get(TextureHandle h) const;

    void remove(TextureHandle h);

    size_t count() const;

private:
    ObjectPool<gr::texture> m_pool;
};



