#pragma once

#include "gr-base/core/default.hpp"
#include "gr-base/core/geometry/default.hpp"
#include "gr-base/ecs/default.hpp"

#include <gr-math/types.hpp>
#include <map>
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

    std::vector<GameObjectHandle> m_markToRender;

    std::vector<std::tuple<ComponentHandle, BatchID>> m_instances;

    std::map<MaterialID, std::map<MeshID, std::vector<ComponentHandle>>> m_queueFirstCmd;

    std::map<MaterialID, std::map<MeshID, std::vector<Matrix4x4>>> m_queueInstancingCmd;

    std::map<uint64_t, std::vector<RenderCommand>> m_renderable;

    void insert(GameObjectHandle handle);

    void remove(GameObjectHandle handle);

    void update();

    void renderer();

    void setup(std::vector<GameObjectHandle> objects);
};


