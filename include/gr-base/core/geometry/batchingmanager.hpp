#pragma once

#include "gr-base/core/geometry/batching.hpp"

#include "default.hpp"

class BatchingManager
{
public:
    static Batching *Create(const std::string &name, std::vector<Mesh> *meshlist);

    static Batching *Find(const std::string &name);

    static void Destroy(Batching *batching);

    static void Render();

private:
    static BatchingManager &GetInstance();

    std::unordered_map<std::string, std::unique_ptr<Batching>> m_batchingmap;

    std::unordered_map<Batching *, const std::string &> m_batchingnamemap;

    std::vector<Batching *> m_batchinglist;

    BatchingManager();
    ~BatchingManager();

    Batching *create(const std::string &name, std::vector<Mesh> *meshlist);

    Batching *find(const std::string &name);

    void destroy(Batching *batching);

    void render();
};


