#include "gr-base/ecs/componentRegistry.hpp"

#include <algorithm>

ComponentRegistry &ComponentRegistry::getInstance()
{
    static ComponentRegistry instance;
    return instance;
}

BaseChunk *ComponentRegistry::Find(const std::string &name)
{
    return getInstance().findScript(name.c_str());
}

void ComponentRegistry::Clear()
{
    getInstance().clear();
}

std::unordered_map<std::string, std::tuple<std::unique_ptr<BaseChunk>, bool>> &ComponentRegistry::GetAllChunkComponent()
{
    return getInstance().m_scriptChunk;
}

BaseChunk *ComponentRegistry::findScript(const char *name) const
{
    auto it = m_scriptChunk.find(name);
    if (it != m_scriptChunk.end())
    {
        return &(*std::get<0>(it->second));
    }
    return nullptr;
}

void ComponentRegistry::init()
{
    for (auto chunk : chunkList)
    {
        chunk->init();
    }
}

void ComponentRegistry::update()
{
    for (auto chunk : chunkList)
    {
        chunk->update();
    }
}

void ComponentRegistry::clear()
{
    for (auto it = m_scriptChunk.begin(); it != m_scriptChunk.end();)
    {
        auto &&[pointer, engine] = it->second;

        if (!engine)
        {
            std::cout << "Clean: " << it->first << std::endl;
            
            chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), &(*pointer)), chunkList.end());
            it = m_scriptChunk.erase(it);
        } else
        {
            it++;
        }
    }
}
