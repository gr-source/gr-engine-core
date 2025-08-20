#include "gr-base/core/geometry/batchingmanager.hpp"

// public
BatchID BatchingManager::Create(const std::string &name, std::vector<Mesh> *meshlist)
{
    return GetInstance().create(name, meshlist);
}

BatchID BatchingManager::Find(const std::string &name)
{
    return GetInstance().find(name);
}

Batching *BatchingManager::Get(BatchID id)
{
    return GetInstance().get(id);
}

void BatchingManager::Destroy(BatchID id)
{
    return GetInstance().destroy(id);
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

BatchingManager::BatchingManager() : m_count(0)
{}

BatchingManager::~BatchingManager()
{}

BatchID BatchingManager::create(const std::string &name, std::vector<Mesh> *meshlist)
{
    if (m_freeID.empty())
        resize(1024);

    BatchID id = m_freeID.back();
    m_freeID.pop_back();

    Batching *batching = &m_entry[id];
    new (batching) Batching(meshlist);

    batching->name = name;

    m_namemaptobatchid[name] = id;

    m_count++;
    return id;
}

BatchID BatchingManager::find(const std::string &name)
{
    auto it = m_namemaptobatchid.find(name);
    if (it == m_namemaptobatchid.end())
        return InvalidBatch;

    return it->second;
}

Batching *BatchingManager::get(BatchID id)
{
    return &m_entry[id];
}

void BatchingManager::destroy(BatchID id)
{
    BatchID lastID = m_count - 1;

    Batching *current = m_entry.data() + id;
    m_namemaptobatchid.erase(current->name);

    current->~Batching();

    Batching *last = m_entry.data() + lastID;

    new (current) Batching(*last);

    m_count--;
}

void BatchingManager::render()
{
    for (BatchID id=0;id<m_count;id++)
        m_entry[id].render();
}

void BatchingManager::resize(const std::size_t &size)
{
    auto oldsize = m_entry.size();
    auto newsize = oldsize + size;

    m_entry.resize(newsize, Batching(nullptr));

    auto temp = newsize;
    while ((temp--) > oldsize)
    {
        Batching &batching = m_entry[temp];
        // batching.name = nullptr;

        m_freeID.push_back(temp);
    }
}


