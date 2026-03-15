#pragma once

#include "gr-base/core/default.hpp"
#include "gr-base/core/geometry/default.hpp"
#include "gr-base/ecs/default.hpp"

#include <gr-math/types.hpp>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct RenderCommand
{
    MaterialID materialID;
    MeshID meshID;
    ComponentHandle transformID;
};

class RenderSystem
{
public:
    static void SetInstance(RenderSystem* renderSystem);

    static void Insert(GameObjectHandle handle);

    static void Remove(GameObjectHandle handle);

    static void Update();

    static void Renderer();

private:
    static RenderSystem* s_RenderSystem;

    std::unordered_set<GameObjectHandle> m_markToRender;

    std::unordered_map<MaterialID, std::unordered_map<MeshID, std::vector<Matrix4x4>>> m_queueFirstCmd;

    // std::unordered_map<GameObjectHandle, std::vector<RenderCommand>> m_renderable;

    std::unordered_map<uint64_t, std::vector<ComponentHandle>> m_renderable;

    std::unordered_map<GameObjectHandle, uint64_t> m_objectsToRenderKey;

    void insert(GameObjectHandle handle);

    void remove(GameObjectHandle gameObjectID);

    void update();

    void renderer();

    void setup(std::unordered_set<GameObjectHandle> objects);
};


