#pragma once

#include "gr-base/core/geometry/batching.hpp"

extern "C"
{
#include "gr-base/externals/array.h"
}

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

    array* m_poolBatches;

    BatchID m_freeID;

    BatchID m_count;

    BatchingManager();
    ~BatchingManager();

    BatchID create(size_t key, MeshID mesheID, MaterialID materialID);

    BatchID find(size_t key);

    Batching &get(BatchID id);

    void destroy(BatchID id);

    void render();

    void addFreeID(BatchID first);
};
/*
#pragma once

#include "gr-base/core/geometry/batching.hpp"

#include "default.hpp"

using BatchID = std::size_t;

constexpr BatchID InvalidBatch = std::numeric_limits<BatchID>::max();

class BatchingManager
{
public:
    static BatchID Create(const std::string &name, std::vector<Mesh> *meshlist);

    static BatchID Find(const std::string &name);

    static void Destroy(BatchID id);

    static Batching *Get(BatchID id);

    static void Render();

private:
    static BatchingManager &GetInstance();

    // std::unordered_map<std::string, std::unique_ptr<Batching>> m_batchingmap;
    //
    // std::unordered_map<Batching *, const std::string &> m_batchingnamemap;
    //
    // std::vector<Batching *> m_batchinglist;

    Batching *m_entry;

    std::size_t m_count;

    std::size_t m_size;

    std::vector<BatchID> m_freeID;

    BatchingManager();
    ~BatchingManager();

    BatchID create(const std::string &name, std::vector<Mesh> *meshlist);

    BatchID find(const std::string &name);

    void destroy(BatchID id);

    Batching *get(BatchID id);

    void render();

    void resize(std::size_t size);
};
*/

