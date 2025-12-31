#include "gr-base/managers/batchingmanager.hpp"

BatchingManager* BatchingManager::s_BatchingManager = nullptr;

// public
void BatchingManager::Initialize()
{
    if (s_BatchingManager == nullptr)
        s_BatchingManager = new BatchingManager();
}

void BatchingManager::Shutdown()
{
    if (s_BatchingManager != nullptr)
        delete s_BatchingManager;
    s_BatchingManager = nullptr;
}

BatchID BatchingManager::Create(const std::string &name, const std::vector<MeshID> &meshes)
{
    return s_BatchingManager->create(name, meshes);
}

BatchID BatchingManager::Find(const std::string &name)
{
    return s_BatchingManager->find(name);
}

Batching &BatchingManager::Get(BatchID id)
{
    return s_BatchingManager->get(id);
}

void BatchingManager::Destroy(BatchID id)
{
    return s_BatchingManager->destroy(id);
}

void BatchingManager::Render()
{
    return s_BatchingManager->render();
}

// private
BatchingManager::BatchingManager() : m_count(0)
{}

BatchingManager::~BatchingManager()
{}

BatchID BatchingManager::create(const std::string &name, const std::vector<MeshID> &meshes)
{
    if (m_freeID.empty())
        resize(1024);

    BatchID id = m_freeID.back();
    m_freeID.pop_back();

    Batching *batching = &m_entry[id];
    new (batching) Batching(meshes);

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

Batching &BatchingManager::get(BatchID id)
{
    return m_entry[id];
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

    m_entry.resize(newsize, Batching({}));

    auto temp = newsize;
    while ((temp--) > oldsize)
    {
        Batching &batching = m_entry[temp];
        // batching.name = nullptr;

        m_freeID.push_back(temp);
    }
}


