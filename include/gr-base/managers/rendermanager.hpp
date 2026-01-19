#pragma once

#include "gr-base/core/default.hpp"
#include "gr-base/core/geometry/default.hpp"
#include "gr-base/managers/octreemanager.hpp"
#include "gr-base/script/default.hpp"

#include <gr-render/gCommon.h>
#include <unordered_map>

class RenderManager
{
public:
    static void Initialize();

    static void Shutdown();

    static void Insert(GameObjectID id);

    static void Remove(GameObjectID id);

    static void MarkDirt(GameObjectID id, const Vector3& position);

    static void PreRender(const Camera *camera);

    static void Render();

    static void PosRender();

private:
    static RenderManager* s_RenderManager;

    std::unordered_map<GameObjectID, NodeID> m_objectInOctree;

    void insert(GameObjectID id);

    BatchID GetBatchID(MeshID meshID, MaterialID materialID) const;

    void remove(GameObjectID id);

    void markDirt(GameObjectID id, const Vector3& position);

    void preRender(const Camera *camera);

    void render();

    void posRender();

    void setuplight(grr::gShader *shader);
};


