#include "gr-base/managers/batchingmanager.hpp"
#include "gr-base/core/geometry/default.hpp"
#include "gr-base/externals/array.h"

#include <iostream>

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

BatchID BatchingManager::Create(size_t key, MeshID mesheID, MaterialID materialID)
{
    return s_BatchingManager->create(key, mesheID, materialID);
}

BatchID BatchingManager::Find(size_t key)
{
    return s_BatchingManager->find(key);
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
BatchingManager::BatchingManager() : m_poolBatches(nullptr), m_freeID(BATCHID_INVALID), m_count(0)
{
    m_poolBatches = array_create(1024, sizeof(BatchWrapper));

    addFreeID(0);
}

BatchingManager::~BatchingManager()
{}

BatchID BatchingManager::create(size_t key, MeshID mesheID, MaterialID materialID)
{
    if (m_batchCache.find(key) != m_batchCache.end())
        return InvalidBatch;

    BatchID id = m_freeID;
    if (id == BATCHID_INVALID)
    {
        std::cerr << "Need resize " << std::endl;
        return BATCHID_INVALID;
    }

    std::cout << "BatchingManager::Create " << key << " | " << mesheID << " | " << materialID << " | " << id << std::endl;

    m_batchCache[key] = id;

    BatchWrapper* wrapper = (BatchWrapper*)m_poolBatches->vector;
    Batching* batch = &wrapper[id].batch;

    new (batch) Batching();
    batch->build(mesheID, materialID);

    m_count++;

    return id;
}

BatchID BatchingManager::find(size_t key)
{
    return m_batchCache.find(key) != m_batchCache.end() ? m_batchCache[key] : BATCHID_INVALID;
}

Batching &BatchingManager::get(BatchID id)
{
    BatchWrapper* wrapper = (BatchWrapper*)m_poolBatches->vector;

    return wrapper[id].batch;
}

void BatchingManager::destroy(BatchID id)
{
    BatchWrapper* wrapper = (BatchWrapper*)m_poolBatches->vector;
    Batching* batch = &wrapper[id].batch;
    batch->~Batching();

    wrapper->next = m_freeID;
    m_freeID = id;

    m_count--;
}

void BatchingManager::render()
{
    BatchWrapper* wrapper = (BatchWrapper*)m_poolBatches->vector;

    for (BatchID i=0;i<m_count;i++)
        wrapper[i].batch.render();
}

void BatchingManager::addFreeID(BatchID first)
{
    BatchWrapper* wrapper = (BatchWrapper*)m_poolBatches->vector;

    BatchID lastBatch = m_poolBatches->length - 1;
    for (BatchID id=first;id<lastBatch;id++)
        wrapper[id].next = id + 1;

    wrapper[lastBatch].next = m_freeID;
    m_freeID = first;
}


