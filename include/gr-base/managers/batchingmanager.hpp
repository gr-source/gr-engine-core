#pragma once

#include "gr-base/core/geometry/batching.hpp"

extern "C"
{
#include "gr-base/externals/array.h"
}

#include <unordered_map>

typedef struct BatchWrapper
{
    BatchID next;
    Batching batch;
} BatchWrapper;

class BatchingManager
{
public:
    static void Initialize();

    static void Shutdown();

    static BatchID Create(size_t key, MeshID mesheID, MaterialID materialID);

    static BatchID Find(size_t key);

    static Batching &Get(BatchID id);

    static void Destroy(BatchID id);

    static void Render();

private:
    static BatchingManager* s_BatchingManager;

    std::unordered_map<size_t, BatchID> m_batchCache;

    array* m_pool;

    BatchID m_freeID;

    BatchID m_count;

    BatchingManager();
    ~BatchingManager();

    BatchID create(size_t key, MeshID mesheID, MaterialID materialID);

    BatchID find(size_t key);

    BatchID allocate();

    Batching &get(BatchID id);

    void destroy(BatchID id);

    void render();

    void realloc(BatchID size);

    void addFreeID(BatchID first);
};


