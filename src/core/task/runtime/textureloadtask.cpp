#include "gr-base/core/task/runtime/textureloadtask.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/console.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "gr-base/core/stb/stb_image.h"

#include <gr-render/gTexture.h>

TextureLoadTask::TextureLoadTask(const std::string &path, TextureFunc func) : m_path(path), m_pixels(nullptr)
{
    addFunction(func);
}

void TextureLoadTask::addFunction(TextureFunc func)
{
    return m_functions.push_back(func);
}

void TextureLoadTask::invoke(gr::gTexture *texture)
{
    for (auto func : m_functions)
        func(texture);
}

gr::gTexture* TextureLoadTask::doInBackground()
{
    bool isHDR = stbi_is_hdr(m_path.c_str());

    int ch;
    if (isHDR)
    {
        m_pixels = std::unique_ptr<char>((char *)stbi_loadf(m_path.c_str(), &width, &height, &ch, 0));
    } else {
        m_pixels = std::unique_ptr<char>((char *)stbi_load(m_path.c_str(), &width, &height, &ch, 0));
    }

    if (!m_pixels)
    {
        Console::LogError("[ TextureLoadTask ] Fail to load texture '%s'", m_path.c_str());
        AssetsManager::RemoveTask(m_path);
        return nullptr;
    }

    switch (ch)
    {
    case 1:
        format = TextureFormat_RED;
        break;
    case 2:
        format = isHDR ? TextureFormat_RG16F : TextureFormat_RG;
        break;
    case 3:
        format = isHDR ? TextureFormat_RGB16F : TextureFormat_RGB;
        break;
    case 4:
        format = isHDR ? TextureFormat_RGBA16F : TextureFormat_RGBA;
        break;
    default:
        return nullptr;
    }

    return AssetsManager::CreateTexture(m_path);
}

void TextureLoadTask::onPostExecute(Result<gr::gTexture*> result)
{
    auto texture = result.value;
    if (texture == nullptr)
    {
        Console::LogError("[!] The texture '%s' could not be loaded.", m_path.c_str());
    } else
    {
        texture->set_filtering(gTextureFlags_Filter_Trilinear);
        texture->set_texture(gTextureFlags_Texture);
        texture->set_format(format);
        texture->generate_mipmaps();

        texture->updateBuffer(width, height, m_pixels.get());

        invoke(texture);

        Console::LogInfo("[ TextureLoadTask ] Texture loaded: %s", m_path.c_str());
    }

    AssetsManager::RemoveTask(m_path);
}












