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

BaseChunk *ComponentRegistry::findScript(const std::string &name) const
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
        chunk->initall(chunk);
    }
}

void ComponentRegistry::update()
{
    for (auto chunk : chunkList)
    {
        chunk->updateall(chunk);
    }
}

std::vector<std::string> ComponentRegistry::getScriptRequestReinstance() const
{
    return m_scriptToRestart;
}

void ComponentRegistry::removeScriptNeedsUpdate(const std::string &name)
{
    m_scriptToRestart.erase(std::remove(m_scriptToRestart.begin(), m_scriptToRestart.end(), name), m_scriptToRestart.end());
}

void ComponentRegistry::CheckScriptRemoved()
{// m_scriptRegister

    for (auto it = m_scriptChunk.begin(); it != m_scriptChunk.end();)
    {
        if (!m_scriptRegister[it->first] && !std::get<1>(it->second))
        {
            auto chunkBase = &(*std::get<0>(it->second));

            m_scriptStride.erase(it->first);

            chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), chunkBase), chunkList.end());

            std::cout << "Unregistry: " << it->first << std::endl;

            it = m_scriptChunk.erase(it);
        } else
        {
            it++;
        }
    }

    m_scriptRegister.clear();
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

bool ComponentRegistry::isCanRegister(const std::string &name)
{
    return m_scriptChunk.find(name) == m_scriptChunk.end();
}

void ComponentRegistry::removeChunkInvector(BaseChunk *baseChunk)
{
    chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), baseChunk), chunkList.end());
}

void ComponentRegistry::checkScriptStride(const std::string &name, std::size_t stride)
{
    // script to remove
    {
        if (m_scriptRegister.find(name) == m_scriptRegister.end())
            m_scriptRegister.emplace(name, true);
    }


    auto it = m_scriptStride.find(name);
    if (it != m_scriptStride.end())
    {
        if (stride != it->second)
        {
            std::cout << "Resize: " << name << std::endl;
            m_scriptToRestart.push_back(name);

            auto &&[chunkBase, isinternal] = m_scriptChunk[name];
            if (chunkBase != nullptr)
            {
                removeChunkInvector(chunkBase.get());
            }

            m_scriptChunk.erase(name);

            m_scriptStride[name] = stride;
        }
    }
}

BaseChunk * ComponentRegistry::registryScript(const std::string &name, std::unique_ptr<BaseChunk> ptr, std::size_t stride, bool isInternal)
{
    std::cout << "Registry: " << name << std::endl;

    m_scriptChunk.emplace(name, std::make_tuple(std::move(ptr), isInternal));

    auto baseChunk = &(*std::get<0>(m_scriptChunk[name]));

    m_scriptStride.emplace(name, stride);

    chunkList.push_back(baseChunk);

    return baseChunk;
}



