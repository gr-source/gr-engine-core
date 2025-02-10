#include "gr-base/core/task/textureTask.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/event.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "gr-base/core/stb/stb_image.h"

#include <gr-render/gTexture.h>

TextureTask::TextureTask(const std::string &path, TextureFunc func) : m_pixels(nullptr)
{
    if (func)
        m_functions.push_back(func);

    Threadpool::AddTask(&TextureTask::doInBackground, this, path);
}

void TextureTask::addFunction(TextureFunc func)
{
    m_functions.push_back(func);
}

void TextureTask::invoke(grr::gTexture *texture)
{
    for (auto func : m_functions)
    {
        func(texture);
    }
}

void TextureTask::upload(std::string path, int width, int height, grr::TextureFormat format)
{
    auto texture = AssetsManager::CreateTexture(path);
    texture->updateBuffer(width, height, grr::gTextureFlags_MipMaps | grr::gTextureFlags_Filter_Bilinear, format, m_pixels.get());

    invoke(texture);

    Console::LogInfo("[ TextureTask ] Texture loaded: %s", path.c_str());

    AssetsManager::RemoveTask(path);
}

void TextureTask::doInBackground(std::string path)
{
    int width, height, ch;
    m_pixels = std::unique_ptr<char>((char *)stbi_load(path.c_str(), &width, &height, &ch, 0));
    if (!m_pixels)
    {
        return Console::LogError("[ TextureTask ] Fail to load texture '%s'", path.c_str());
    }

    grr::TextureFormat format;
    switch (ch)
    {
    case 1:
        // return Texture::eRED;
    case 2:
        // return Texture::eRG;
    case 3:
        format = grr::TextureFormat_SRGB;
        break;
    case 4:
        format = grr::TextureFormat_SRGBA;
        break;
    default:
        break;
    }

    Event::Add(&TextureTask::upload, this, path, width, height, format);
}
