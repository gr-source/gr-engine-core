#include "gr-base/ecs/componentRegistry.hpp"

#include <algorithm>

// private
ComponentRegistry &ComponentRegistry::GetInstance()
{
    static ComponentRegistry instance;
    return instance;
}

// public
IComponentStorage *ComponentRegistry::Find(const std::string &name)
{
    return GetInstance().find(name);
}

std::vector<std::string> ComponentRegistry::GetScriptRequestReinstance()
{
    return std::move(GetInstance().m_scriptToRestart);
}

void ComponentRegistry::Update()
{
    return GetInstance().update();
}

void ComponentRegistry::CheckScriptRemoved()
{
    return GetInstance().check_script_removed();
}

void ComponentRegistry::Clear()
{
    GetInstance().clear();
}

std::vector<ComponentInfo *> &ComponentRegistry::GetComponentInfoList()
{
    return GetInstance().m_componentinfolist;
}

IComponentStorage *ComponentRegistry::find(const std::string &name) const
{
    auto it = m_componentstoragemap.find(name);
    if (it == m_componentstoragemap.end())
        return nullptr;

    return it->second.get();
}

void ComponentRegistry::init()
{
    for (auto &componentInfo : m_componentinfolist)
    {
        auto storage = componentInfo->storage;

        storage->initall(storage);
    }
}

void ComponentRegistry::update()
{
    for (auto &chunk : m_componentinfolist)
    {
        auto storage = chunk->storage;

        storage->updateall(storage);
    }
}

void ComponentRegistry::removeScriptNeedsUpdate(const std::string &name)
{
    m_scriptToRestart.erase(std::remove(m_scriptToRestart.begin(), m_scriptToRestart.end(), name), m_scriptToRestart.end());
}

void ComponentRegistry::check_script_removed()
{
    std::vector<std::string> unknown;
    for (auto &&[name, storage] : m_componentstoragemap)
    {
        if (!m_componentinfomap[name]->is_engine && m_scriptRegister.find(name) == m_scriptRegister.end())
        {
            unknown.push_back(name);
        }
    }

    for (auto &name : unknown)
    {
        remove_component(name);
    }
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
    */

    m_scriptRegister.clear();
}

void ComponentRegistry::clear()
{
    auto temp = m_componentinfolist;
    for (auto &componentInfo : temp)
    {
        if (componentInfo->is_engine)
            continue;

        m_componentinfolist.erase(std::remove(m_componentinfolist.begin(), m_componentinfolist.end(), componentInfo), m_componentinfolist.end());

        m_componentstoragemap.erase(componentInfo->name);
        m_componentinfomap.erase(componentInfo->name);
    }
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

bool ComponentRegistry::isCanRegister(const std::string &name)
{
    return m_componentstoragemap.find(name) == m_componentstoragemap.end();
}

void ComponentRegistry::remove_component(const std::string &name)
{
    auto componentInfo = m_componentinfomap[name].get();
    m_componentinfolist.erase(std::remove(m_componentinfolist.begin(), m_componentinfolist.end(), componentInfo), m_componentinfolist.end());

    m_componentinfomap.erase(name);
    m_componentstoragemap.erase(name);
}

void ComponentRegistry::checkScriptStride(const std::string &name, std::size_t stride)
{
    // script to remove
    {
        if (m_scriptRegister.find(name) == m_scriptRegister.end())
            m_scriptRegister.emplace(name);
    }

    auto it = m_componentinfomap.find(name);
    if (it != m_componentinfomap.end())
    {
        if (stride != it->second->stride)
        {
            m_scriptToRestart.push_back(name);

            remove_component(name);
        }
    }

    /*
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

IComponentStorage * ComponentRegistry::registryScript(const std::string &name, std::unique_ptr<IComponentStorage> ptr, std::size_t stride, bool isInternal)
{
    m_componentstoragemap.emplace(name, std::move(ptr));
    
    {
        auto &&[it, added] = m_componentinfomap.emplace(name, std::make_unique<ComponentInfo>());

        auto componentInfo = it->second.get();
        componentInfo->name = name;
        componentInfo->stride = stride;
        componentInfo->is_engine = isInternal;
        componentInfo->storage = m_componentstoragemap[name].get();

        m_componentinfolist.push_back(componentInfo);
    }

    return m_componentstoragemap[name].get();
}



