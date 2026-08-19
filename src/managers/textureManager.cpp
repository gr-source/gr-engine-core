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

TextureID TextureManager::create()
{
    TextureID h = m_pool.emplace();

    auto& texture = m_pool.get(h);
    texture.create();

    return h;
}

gr::texture *TextureManager::get(TextureID h) const
{
    return m_pool.try_get(h);
}

void TextureManager::remove(TextureID h)
{
    gr::texture* texture = m_pool.try_get(h);
    if (texture == nullptr)
        return;

    texture->destroy();

    m_pool.destroy(h);
}

size_t TextureManager::count() const
{
    return m_pool.count();
}



