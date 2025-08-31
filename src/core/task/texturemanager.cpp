#include "gr-base/core/task/texturemanager.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/file/file.hpp"

TextureManager::TextureManager()
{}

grr::gTexture *TextureManager::create(const std::string &name)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto &&[it, added] = m_texturemap.emplace(name, std::make_unique<grr::gTexture>());
    if (added)
    {
        m_texturenamemap.emplace(it->second.get(), name);
    }

    return it->second.get();
}

void TextureManager::load(const std::string &path, std::function<void (grr::gTexture *)> func)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Check texture has in main list
    {
        auto it = m_texturemap.find(path);
        if (it != m_texturemap.end())
        {
            if (func == nullptr)
                return;
            return func(&(*it->second));
        }
    }

    // Check texture is loading
    {
        auto it = m_texturetaskmap.find(path);
        if (it != m_texturetaskmap.end())
        {
            if (func == nullptr)
                return;
            return it->second->addFunction(func);
        }
    }

    if (!gfile::exists(path))
    {
        return Console::LogError("[ AssetsManager::Texture ] File not found: %s", path.c_str());
    }
    
    // Create task to load texture
    m_texturetaskmap.emplace(path, std::make_unique<TextureLoadTask>(path, func));
}

bool TextureManager::remove_task(const std::string &name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_texturetaskmap.find(name);
    if (it == m_texturetaskmap.end())
        return false;

    m_texturetaskmap.erase(it);
    return true;
}

const char *TextureManager::getname(grr::gTexture *texture)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_texturenamemap.find(texture);
    if (it == m_texturenamemap.end())
    {
        return nullptr;
    }
    return it->second.c_str();
}

std::size_t TextureManager::getcount()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_texturemap.size();
}


