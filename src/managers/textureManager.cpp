#include "gr-base/managers/textureManager.hpp"

#include <gr-render/texture.hpp>

TextureManager::TextureManager()
{}

TextureManager::~TextureManager()
{
    gr::texture* textures = sparseSet.data();
    for (size_t i=0;i<sparseSet.count();i++)
    {
        gr::texture& texture = textures[i];
        texture.destroy();
    }
}

TextureID TextureManager::create(const TextureDesc& desc)
{
    TextureID h = sparseSet.emplace();

    auto& texture = sparseSet.get(h);
    texture.create();

    texture.width = desc.width;
    texture.height = desc.height;

    texture.filter = desc.filter;
    texture.format = desc.format;
    texture.type = desc.type;
    texture.wrap = desc.wrap;

    texture.bind();
    texture.applyTextureSettings();
    texture.applyTextureBuffer(desc.pixels);
    texture.unbind();

    return h;
}

gr::texture *TextureManager::get(TextureID id) const
{
    return sparseSet.try_get(id);
}

void TextureManager::remove(TextureID id)
{
    gr::texture* texture = sparseSet.try_get(id);
    if (texture == nullptr)
        return;

    texture->destroy();

    sparseSet.destroy(id);
}

size_t TextureManager::count() const
{
    return sparseSet.count();
}



