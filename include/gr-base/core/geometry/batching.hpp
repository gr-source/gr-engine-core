#pragma once

#include "gr-base/script/default.hpp"

#include <gr-render/gRender.h>

#include "default.hpp"
#include "material.hpp"
#include "boundbox.hpp"

#include <vector>
#include <memory>

enum BatchingFlags : uint32_t {
    BatchingFlags_None           = 1 << 0,
    BatchingFlags_HasIndices     = 1 << 1,
    BatchingFlags_BuildMesh      = 1 << 2,
};

using BatchingFlags_ = uint32_t;

class BatchingDraw {
public:
    BatchingDraw();
    ~BatchingDraw();

    void build(Mesh *mesh, int entityID);

    void draw();

    void updatePBO(const Matrix4x4 *matrix, std::size_t count);

    grr::gVertexArray *m_vertexArray;

    grm::u32 m_instanceCount;

    grm::u32 m_indexCount;

    grm::u32 m_VBO;

    grm::u32 m_EBO;

    grm::u32 m_PBO;

};


class Batching {
public:
    // ***** Static Method ***** //
    static void Initialize();

    static void Shutdown();

    static void RenderShadowPass();

    static grm::size GetBatchCount();

    static Batching *FindBatchByName(const std::string &name);

    template <typename... Args>
    static Batching *CreateBatch(const std::string &key, Args &&...args);

    static void RemoveBatch(Batching *batching);

    static void RenderBatches();

    // ***** Class Method ***** //

    Batching(std::vector<Material *> *materials, BatchingFlags_ flags = 0);
    ~Batching();

    void buildMeshe(std::vector<Mesh> *meshList);

    void insert(Transform *transform);

    void remove(Transform *transform);

    void draw();

    bool isValid() const;

    bool isVisible() const;

private:
    static std::unordered_map<std::string, std::unique_ptr<Batching>> mBatchInstanceMap;

    static std::unordered_map<Batching *, std::string> mBatchRefToNameMap;

    static std::vector<Batching *> mBatchRefList;

    static grr::gShader *s_shadowShader;

    std::vector<std::unique_ptr<BatchingDraw>> m_drawList;

    std::vector<Material *> *m_materials;

    std::vector<Transform *> m_models;

    Boundbox m_boundbox;

    int meshCount;

    BatchingFlags_ m_flags;

    void calcboundbox(const Mesh *mesh);
};

template <typename... Args>
inline Batching *Batching::CreateBatch(const std::string &key, Args &&...args) {
    auto &&[it, added] = mBatchInstanceMap.emplace(key, std::make_unique<Batching>(std::forward<Args>(args)...));

    auto ptr = &(*it->second);
    
    mBatchRefList.push_back(ptr);

    mBatchRefToNameMap.emplace(ptr, key);

    return &(*ptr);
}
