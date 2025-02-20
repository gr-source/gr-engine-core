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
    texture->updateBuffer(width, height, grr::gTextureFlags_MipMaps | grr::gTextureFlags_Filter_Trilinear, format, m_pixels.get());

    invoke(texture);

    Console::LogInfo("[ TextureTask ] Texture loaded: %s", path.c_str());

    AssetsManager::RemoveTask(path);
}

void TextureTask::doInBackground(std::string path)
{
    bool isHDR = stbi_is_hdr(path.c_str());

    int width, height, ch;

    if (isHDR)
    {
        m_pixels = std::unique_ptr<char>((char *)stbi_loadf(path.c_str(), &width, &height, &ch, 0));
    } else {
        m_pixels = std::unique_ptr<char>((char *)stbi_load(path.c_str(), &width, &height, &ch, 0));
    }

    if (!m_pixels)
    {
        Console::LogError("[ TextureTask ] Fail to load texture '%s'", path.c_str());
        return AssetsManager::RemoveTask(path);
    }

    grr::TextureFormat format;
    switch (ch)
    {
    case 1:
        format = grr::TextureFormat_RED;
        break;
    case 2:
        format = isHDR ? grr::TextureFormat_RG16F : grr::TextureFormat_RG;
        break;
    case 3:
        format = isHDR ? grr::TextureFormat_RGB16F : grr::TextureFormat_RGB;
        break;
    case 4:
        format = isHDR ? grr::TextureFormat_RGBA16F : grr::TextureFormat_RGBA;
        break;
    default:
        std::cout << "Errr: " << ch << std::endl;
        break;
    }

    Event::Add(&TextureTask::upload, this, path, width, height, format);
}
