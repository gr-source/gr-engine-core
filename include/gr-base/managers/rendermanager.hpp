#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/managers/bvhtreemanager.hpp"
#include "gr-base/managers/octreemanager.hpp"
#include "gr-base/script/default.hpp"

#include <gr-render/gCommon.h>
#include <unordered_map>

typedef struct NodeInfo
{
    NodeID id;
    uint8_t type;
} NodeInfo;

class RenderManager
{
public:
    static void SetInstance(RenderManager* renderManager);

    static void Insert(GameObjectHandle id);

    static void Remove(GameObjectHandle id);

    static void SwitchTo(GameObjectHandle handle, bool isStatic);

    static void PreRender(const Camera *camera);

    static void Render();

    static void PosRender();

private:
    static RenderManager* s_RenderManager;

    std::unordered_map<GameObjectHandle, NodeInfo> m_objectInOctree;

    void insert(GameObjectHandle id);

    BatchID GetBatchID(Boundbox& boundbox, MeshID meshID, MaterialID materialID) const;

    void remove(GameObjectHandle handle);

    void switchTo(GameObjectHandle handle, bool isStatic);

    void preRender(const Camera *camera);

    void AsyncBVHTree(const Camera* camera);

    void foreach_bhv(BVHNode* node);

    void render();

    void posRender();

    void setuplight(const Scene* scene, grr::gShader *shader);
};


