#pragma once

#include "gr-base/script/default.hpp"

#include <gr-render/gRender.h>

#include "default.hpp"
#include "material.hpp"
#include "boundbox.hpp"

#include <vector>
#include <memory>

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

struct InstancedData
{
    int entityID;
    Matrix4x4 model;
};

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

    std::vector<Mesh> *m_meshList;

    std::vector<std::unique_ptr<BatchingRender>> m_batchingRenderList;

    std::vector<BatchedModel> m_batchedModelList;

    std::vector<InstancedData> m_instanceList;

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
