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
BatchingManager::BatchingManager() : m_pool(nullptr), m_freeID(BATCHID_INVALID), m_count(0)
{}

BatchingManager::~BatchingManager()
{
    if (m_pool != nullptr)
        array_free(m_pool);
}

BatchID BatchingManager::create(size_t key, MeshID meshID, MaterialID materialID)
{
    BatchID id = allocate();

    m_batchCache[key] = id;

    BatchWrapper* wrapper = (BatchWrapper*)m_pool->vector;
    m_freeID = wrapper[id].next;

    Batching* batch = &wrapper[id].batch;

    new (batch) Batching();
    batch->build(meshID, materialID);

    // std::cout << "[GPU] Batch criado com Mesh key: " << key << " | meshID: " << meshID << " | materialID:  " << materialID << std::endl;

    m_count++;

    return id;
}

BatchID BatchingManager::find(size_t key)
{
    return m_batchCache.find(key) != m_batchCache.end() ? m_batchCache[key] : BATCHID_INVALID;
}

BatchID BatchingManager::allocate()
{
    if (m_freeID == InvalidBatch)
        realloc(512);

    BatchWrapper* wrapper = (BatchWrapper*)m_pool->vector;

    BatchID id = m_freeID;
    m_freeID = wrapper[id].next;

    return id;
}

Batching &BatchingManager::get(BatchID id)
{
    BatchWrapper* wrapper = (BatchWrapper*)m_pool->vector;

    return wrapper[id].batch;
}

void BatchingManager::destroy(BatchID id)
{
    BatchWrapper* wrapper = (BatchWrapper*)m_pool->vector;
    Batching* batch = &wrapper[id].batch;
    batch->~Batching();

    wrapper->next = m_freeID;
    m_freeID = id;

    m_count--;
}

void BatchingManager::render()
{
    BatchWrapper* wrapper = (BatchWrapper*)m_pool->vector;

    for (BatchID i=0;i<m_count;i++)
        wrapper[i].batch.render();
}

void BatchingManager::realloc(BatchID size)
{
    BatchID lastsize = m_pool != nullptr ? m_pool->length : 0;
    BatchID newsize = lastsize + size;

    if (m_pool == nullptr)
    {
        m_pool = array_create(newsize, sizeof(BatchWrapper));
    } else
    {
        array* newPool = array_create(newsize, sizeof(BatchWrapper));

        BatchWrapper* lastWrapperPool = (BatchWrapper*)m_pool->vector;
        BatchWrapper* newWrapperPool = (BatchWrapper*)newPool->vector;

        for (BatchID i=0;i<lastsize;i++)
        {
            BatchWrapper& lastWrapper = lastWrapperPool[i];
            BatchWrapper& newWrapper = newWrapperPool[i];

            new (&newWrapper.batch) Batching(lastWrapper.batch);
        }

        array_free(m_pool);
        m_pool = newPool;
    }

    addFreeID(lastsize);
}

void BatchingManager::addFreeID(BatchID first)
{
    BatchWrapper* wrapper = (BatchWrapper*)m_pool->vector;

    BatchID lastID = m_pool->length - 1;
    for (BatchID id=first;id<lastID;id++)
        wrapper[id].next = id + 1;

    wrapper[lastID].next = m_freeID;
    m_freeID = first;
}


