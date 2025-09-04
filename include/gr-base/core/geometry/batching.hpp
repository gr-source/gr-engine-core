#pragma once

#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/geometry/batchingdynamic.hpp"

class Batching
{
public:
    Batching(const std::vector<MeshID> &meshes);
    ~Batching();

    std::string name;

    BatchingDynamic *insert(Transform *transform, const std::vector<MaterialID> &materials);

    void render();

private:
    std::unordered_map<std::string, std::shared_ptr<BatchingDynamic>> m_batchingdynamicmap;

    std::vector<BatchingDynamic *> m_batchingdynamiclist;

    std::vector<MeshID> m_meshes;
};

/*
#pragma once

#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/geometry/batchingdynamic.hpp"
#include "gr-base/core/geometry/batchingstatic.hpp"

typedef struct Batching
{
    char *name;

    std::vector<Mesh> *meshlist;

    void insert(Transform *transform, const std::vector<Material *> &material);

    void erase(Transform *transform);

    void render();

    // std::unordered_map<std::string, std::unique_ptr<BatchingDynamic>> m_batchingdynamicmap;
    //
    // std::vector<BatchingDynamic *> m_batchingdynamiclist;
    //
    // std::vector<Mesh> *m_meshlist;
    //
    // std::size_t m_count;
} Batching;
*/










