#include "gr-base/core/geometry/batchingmanager.hpp"

#include <algorithm>

// public
Batching *BatchingManager::Create(const std::string &name, std::vector<Mesh> *meshlist)
{
    return GetInstance().create(name, meshlist);
}

Batching *BatchingManager::Find(const std::string &name)
{
    return GetInstance().find(name);
}

void BatchingManager::Destroy(Batching *batching)
{
    return GetInstance().destroy(batching);
}

void BatchingManager::Render()
{
    return GetInstance().render();
}

// private
BatchingManager &BatchingManager::GetInstance()
{
    static BatchingManager instance;
    return instance;
}

BatchingManager::BatchingManager()
{}

BatchingManager::~BatchingManager()
{}

Batching *BatchingManager::create(const std::string &name, std::vector<Mesh> *meshlist)
{
    auto &&[it, added] = m_batchingmap.emplace(name, std::make_unique<Batching>(meshlist));
    if (added)
    {
        m_batchinglist.push_back(it->second.get());

        m_batchingnamemap.emplace(it->second.get(), name);
    }
    return it->second.get();
}

Batching *BatchingManager::find(const std::string &name)
{
    auto it = m_batchingmap.find(name);
    if (it == m_batchingmap.end())
        return nullptr;
    return it->second.get();
}

void BatchingManager::destroy(Batching *batching)
{
    auto it = m_batchingnamemap.find(batching);
    if (it == m_batchingnamemap.end())
        return;

    m_batchinglist.erase(std::remove(m_batchinglist.begin(), m_batchinglist.end(), batching), m_batchinglist.end());

    m_batchingmap.erase(it->second);
    m_batchingnamemap.erase(it);
}

void BatchingManager::render()
{
    for (auto &batching : m_batchinglist)
        batching->render();
}



