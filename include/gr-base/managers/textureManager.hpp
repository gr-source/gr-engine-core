#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

#include <gr-render/default.hpp>

#include <string.h>
#include "gr-base/pool/sparseSet.hpp"

#include <unordered_map>

class TextureManager
{
public:
    TextureManager();
    ~TextureManager();

    TextureID create();

    gr::texture *get(TextureID h) const;

    void remove(TextureID h);

    size_t count() const;

private:
    SparseSet2<gr::texture> m_pool;
};



