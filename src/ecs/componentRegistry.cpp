#include "gr-base/ecs/componentRegistry.hpp"

#include <algorithm>

// public
std::vector<std::string> ComponentRegistry::GetScriptsNeedingReinstance()
{
    return GetInstance().getScriptsNeedingReinstance();
}

void ComponentRegistry::Update()
{
    return GetInstance().update();
}

// private
ComponentRegistry &ComponentRegistry::GetInstance()
{
    static ComponentRegistry instance;
    return instance;
}


std::vector<std::string> ComponentRegistry::getScriptsNeedingReinstance()
{
    return std::move(m_scriptToRestart);
}

IComponentStorage *ComponentRegistry::find(const std::string &name)
{
    auto it = m_componentstorage.find(name);
    if (it != m_componentstorage.end())
    {
        return it->second.get();
    }
    return nullptr;
}

IComponentStorage *ComponentRegistry::Find(const std::string &name)
{
    return GetInstance().find(name);
}

void ComponentRegistry::Clear()
{
    return GetInstance().clear();
}

/*
std::unordered_map<std::string, std::tuple<std::unique_ptr<BaseChunk>, bool>> &ComponentRegistry::GetAllChunkComponent()
{
    return GetInstance().m_scriptChunk;
}
*/
void ComponentRegistry::init()
{
    /*
    for (auto chunk : m_componentstorage)
    {
        chunk->initall(chunk);
    }
    */
}

void ComponentRegistry::update()
{
    /*
    for (auto chunk : chunkList)
    {
        chunk->updateall(chunk);
    }
    */
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
    /*
    for (auto it = m_scriptChunk.begin(); it != m_scriptChunk.end();)
    {
        if (!m_scriptRegister[it->first] && !std::get<1>(it->second))
        {
            auto chunkBase = &(*std::get<0>(it->second));

            m_scriptStride.erase(it->first);

            chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), chunkBase), chunkList.end());

            it = m_scriptChunk.erase(it);
        } else
        {
            it++;
        }
    }

    m_scriptRegister.clear();
    */
}

void ComponentRegistry::clear()
{
    /*
    for (auto it = m_scriptChunk.begin(); it != m_scriptChunk.end();)
    {
        auto &&[pointer, engine] = it->second;

        if (!engine)
        {
            chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), &(*pointer)), chunkList.end());
            it = m_scriptChunk.erase(it);
        } else
        {
            it++;
        }
    }
    */
}
/*
bool ComponentRegistry::isCanRegister(const std::string &name)
{
    return m_scriptChunk.find(name) == m_scriptChunk.end();
}

void ComponentRegistry::removeChunkInvector(BaseChunk *baseChunk)
{
    chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), baseChunk), chunkList.end());
}
*/
void ComponentRegistry::checkScriptStride(const std::string &name, std::size_t stride)
{
    /*
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
    */
}



