#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/script/default.hpp"

#include "default.hpp"

class Batching
{
public:
    Batching();
    ~Batching();

    void insert(ComponentID transform);

    void remove(ComponentID transform);

    void build(MeshID meshID, MaterialID materialID);

    void render();

private:
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
    uint32_t pbo;
    MaterialID materialID;
    uint32_t vertexCount;

    std::vector<ComponentID> instances;
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










