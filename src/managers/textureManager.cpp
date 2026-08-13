#include "gr-base/managers/textureManager.hpp"

#include <gr-render/texture.hpp>

#include <cassert>
#include <cstdlib>

TextureManager::TextureManager()
{}

TextureManager::~TextureManager()
{
    gr::texture* textures = m_pool.data();
    for (size_t i=0;i<m_pool.count();i++)
    {
        gr::texture& texture = textures[i];
        texture.destroy();
    }
}

TextureHandle TextureManager::create()
{
    TextureHandle h = m_pool.create();

    gr::texture* texture = m_pool.get(h);
    texture->create();

    return h;
}

gr::texture *TextureManager::get(TextureHandle h) const
{
    return m_pool.get(h);
}

void TextureManager::remove(TextureHandle h)
{
    gr::texture* texture = m_pool.get(h);
    if (texture == nullptr)
        return;

    texture->destroy();

    m_pool.destroy(h);
}

size_t TextureManager::count() const
{
    return m_pool.count();
}



