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
    BatchingFlags_UploadMaterial = 1 << 3,
};

using BatchingFlags_ = uint32_t;

class Batching {
public:
    // ***** Static Method ***** //
    static void Initialize();

    static void Shutdown();

    static void RenderShadowPass();

    static grm::size GetBatchCount();

    static Batching *FindBatchByName(const std::string &name);

    static void RemoveBatchByName(const std::string &name);

    template <typename... Args>
    static Batching *AddBatch(const std::string &key, Args &&...args);

    static void RenderBatches();

    // ***** Class Method ***** //

    Batching(const std::string &name, std::vector<Material *> *materials, BatchingFlags_ flags = 0);
    ~Batching();

    void buildMeshe(Mesh *mesh);

    void insert(Transform *transform);

    void remove(Transform *transform);

    void draw();

    bool isValid() const;

    bool isVisible() const;

private:
    static std::unordered_map<std::string, std::unique_ptr<Batching>> s_batchedInstances;

    static std::vector<Batching *> s_batchedInstancesList;

    static grr::gShader *s_shadowShader;

    std::vector<Material *> *m_materials;

    std::vector<Transform *> m_models;

    grr::gVertexArray *m_vertexArray;

    grm::u32 m_instanceCount;

    grm::u32 m_indexCount;

    std::string m_name;

    Boundbox m_boundbox;

    grm::u32 m_VBO;

    grm::u32 m_EBO;

    grm::u32 m_PBO;

    BatchingFlags_ m_flags;

    void setupMaterial(grr::gShader *shader);

    void calcboundbox(const Mesh *mesh);

    static void RegisterShaderShadow(grr::gShader *shader);
};

template <typename... Args>
inline Batching *Batching::AddBatch(const std::string &key, Args &&...args) {
    auto &&[it, added] = s_batchedInstances.emplace(key, std::make_unique<Batching>(key, std::forward<Args>(args)...));

    auto ptr = &(*it->second);
    
    s_batchedInstancesList.push_back(ptr);

    return &(*ptr);
}
