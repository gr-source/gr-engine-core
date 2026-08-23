#pragma once

#include "gr-base/core/default.hpp"
#include "gr-base/core/texture.hpp"

#include "gr-base/pool/sparseSet.hpp"

#include <gr-render/default.hpp>

class TextureManager
{
public:
    TextureManager();
    ~TextureManager();

    TextureID create(const TextureDesc& desc);

    gr::texture *get(TextureID h) const;

    void remove(TextureID h);

    size_t count() const;

private:
    SparseSet2<gr::texture> sparseSet;
};



