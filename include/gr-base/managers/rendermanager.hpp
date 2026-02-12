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

    static void Insert(GameObjectID id);

    static void Remove(GameObjectID id);

    static void SwitchTo(GameObjectID handle, bool isStatic);

    static void PreRender(const Camera *camera);

    static void Render();

    static void PosRender();

private:
    static RenderManager* s_RenderManager;

    std::unordered_map<GameObjectID, NodeInfo> m_objectInOctree;

    void insert(GameObjectID id);

    BatchID GetBatchID(Boundbox& boundbox, MeshID meshID, MaterialID materialID) const;

    void remove(GameObjectID handle);

    void switchTo(GameObjectID handle, bool isStatic);

    void preRender(const Camera *camera);

    void foreach_bhv(BVHNode* node);

    void render();

    void posRender();

    void setuplight(const Scene* scene, grr::gShader *shader);
};


