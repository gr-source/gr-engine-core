#pragma once

#include "gr-base/script/default.hpp"

#include <gr-render/gRender.h>

#include "default.hpp"
#include "material.hpp"
#include "boundbox.hpp"

#include <algorithm>
#include <vector>
#include <memory>
#include <map>

#include "batchingRender.hpp"

enum OptimizeFlags : uint8_t
{
    OptimizeFlags_NoOptimize   = 1 << 0,
    OptimizeFlags_InstanceMesh = 1 << 1,
    OptimizeFlags_SaticMesh    = 1 << 2,
};

enum BatchingFlags : uint16_t {
    BatchingFlags_None           = 1 << 0,
    BatchingFlags_BuildMesh      = 1 << 2,
};

using BatchingFlags_ = uint16_t;

struct BatchedModel
{
    std::vector<Material *> *materialList;

    Transform *transform;

    BatchedModel(std::vector<Material *> *_materialList, Transform *_transform) : materialList(_materialList), transform(_transform)
    {}

    ~BatchedModel()
    {
    }
    
    bool operator ==(const Transform *other) const
    {
        return transform == other;
    }
};

struct BatchedInstanced {
    std::vector<Material *> *materialList;

    std::vector<Transform *> transformList;

    BatchedInstanced(std::vector<Material *> *_materialList) : materialList(_materialList)
    {}

    ~BatchedInstanced()
    {
    }
    
    bool operator ==(const Transform *other) const
    {
        return std::find(transformList.begin(), transformList.end(), other) != transformList.end();
    }
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

    Batching(std::vector<Mesh> *meshList);
    ~Batching();

    void buildMeshe();

    void insert(Transform *transform, std::vector<Material *> *materialList);

    void remove(Transform *transform);

    void draw();

    bool isValid() const;

    bool isVisible() const;

    OptimizeFlags optimizeFlags;

private:
    static std::unordered_map<std::string, std::unique_ptr<Batching>> mBatchInstanceMap;

    static std::unordered_map<Batching *, std::string> mBatchRefToNameMap;

    static std::vector<Batching *> mBatchRefList;

    static std::map<grm::u8, grr::gShader *> mShaderOptimize;

    std::unordered_map<std::string, std::tuple<Transform *, OptimizeFlags>> m_optmizeMap;

    std::unordered_map<std::string, std::size_t> m_optmizeCountMap;

    std::unordered_map<Transform *, std::string> m_materialRef;

    std::vector<Mesh> *m_meshList;

    std::vector<std::unique_ptr<BatchingRender>> m_batchingRenderList;

    std::vector<std::unique_ptr<BatchedInstanced>> m_batchingInstRenderList;

    std::vector<std::unique_ptr<BatchingRender>> m_batchingRenderInstancedList;

    std::vector<BatchedModel> m_batchedModelNoOptimizeList;

    std::vector<std::tuple<std::vector<Material *> *, std::vector<Transform *>>> m_modelsInstacedFromRender;

    std::unordered_map<std::string, std::tuple<std::vector<Material *> *, std::vector<Transform *>> *> m_batchedModelInstancedMap;

    Boundbox m_boundbox;

    int meshCount;

    BatchingFlags_ m_flags;

    void calcboundbox(const Mesh *mesh);

    static void AddShaderOptimize(grm::u8 id, grr::gShader *shader);

    static void SetUpdateShaderOptmize(Material *material, grr::gShader *shader, int id);
};

template <typename... Args>
inline Batching *Batching::CreateBatch(const std::string &key, Args &&...args) {
    auto &&[it, added] = mBatchInstanceMap.emplace(key, std::make_unique<Batching>(std::forward<Args>(args)...));

    auto ptr = &(*it->second);
    
    mBatchRefList.push_back(ptr);

    mBatchRefToNameMap.emplace(ptr, key);

    return &(*ptr);
}
