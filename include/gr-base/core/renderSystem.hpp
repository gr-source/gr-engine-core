#pragma once

#include "gr-base/core/default.hpp"
#include "gr-base/core/geometry/default.hpp"
#include "gr-base/ecs/default.hpp"

#include <vector>

class RenderSystem
{
public:
    static void SetInstance(RenderSystem* renderSystem);

    static void Insert(GameObjectHandle handle);

    static void Remove(GameObjectHandle handle);

    static void Update();

private:
    static RenderSystem* s_RenderSystem;

    std::vector<GameObjectHandle> m_markToRender;

    std::vector<std::tuple<ComponentHandle, BatchID>> m_instances;

    void insert(GameObjectHandle handle);

    void remove(GameObjectHandle handle);

    void update();

    void setup(std::vector<GameObjectHandle> objects);
};


