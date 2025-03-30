#pragma once

#include "gr-base/script/default.hpp"

#include <array>
#include <gr-render/gRender.h>

#include "default.hpp"
#include "gr-base/script/transform.hpp"
#include "material.hpp"
#include "boundbox.hpp"

#include <algorithm>
#include <gr-render/gShader.h>
#include <vector>
#include <memory>

#include <unordered_set>
#include "batchingRender.hpp"

struct Instance
{
    Transform *transform;
    
    std::vector<Material *> *materialList;

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

private:
    static std::unordered_map<std::string, std::unique_ptr<Batching>> mBatchInstanceMap;

    static std::unordered_map<Batching *, std::string> mBatchRefToNameMap;

    static std::vector<Batching *> mBatchRefList;

    std::vector<BatchingRender> m_batchingRender;
    
    std::unordered_set<Transform *> m_instanceMap;

    std::vector<Instance> m_instanceList;

    std::vector<Mesh> *m_meshList;

    Boundbox m_boundbox;

    void calcboundbox(const Mesh *mesh);

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
