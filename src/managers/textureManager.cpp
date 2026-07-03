#include "gr-base/managers/textureManager.hpp"

#include "gr-base/externals/handle.h"

#include <gr-render/gTexture.h>

#include <cassert>
#include <cstdlib>

TextureManager::TextureManager()
{}

TextureManager::~TextureManager()
{
    gr::Texture* textures = m_pool.data();
    for (size_t i=0;i<m_pool.count();i++)
    {
        gr::Texture& texture = textures[i];
        texture.destroy();
    }
}

TextureHandle TextureManager::create(AssetHash hash)
{
    TextureHandle h = m_pool.create();

    gr::Texture* texture = m_pool.get(h);
    texture->create();

    if (hash != INVALID_HASH64)
        m_hashmap.emplace(hash, h);

    return h;
}

gr::Texture *TextureManager::get(TextureHandle h) const
{
    return m_pool.get(h);
}

void TextureManager::remove(TextureHandle h)
{
    gr::Texture* texture = m_pool.get(h);
    if (texture == nullptr)
        return;

    texture->destroy();

    m_pool.destroy(h);
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
    return m_pool.count();
}



