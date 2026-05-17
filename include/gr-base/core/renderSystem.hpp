#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/core/materialInstance.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/managers/default.hpp"

#include <gr-math/types.hpp>
#include <gr-render/shader.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class RenderBatch
{
public:
    std::vector<ComponentHandle> instances;
    std::vector<Matrix4x4> matrices;

    std::shared_ptr<MaterialInstance> materialInstance;

    Mesh* mesh;
    int submeshID;
};

class RenderSystem
{
public:
    static void SetInstance(RenderSystem* renderSystem);

    static void Insert(GameObjectHandle handle);

    static void Remove(GameObjectHandle handle);

    static void Update();

    static void Begin(const Camera* camera);
    
    static void End();

    static void Renderer();

private:
    static RenderSystem* s_RenderSystem;

    std::unordered_set<GameObjectHandle> m_markToRender;

    std::unordered_map<uint64_t, size_t> m_batchIndex;

    std::vector<RenderBatch> m_batches;

    void insert(GameObjectHandle handle);

    void remove(GameObjectHandle gameObjectID);

    void update();

    void begin(const Camera* camera);

    void end();

    void renderer();

    void setup(GameObjectHandle handle);

    void setuplight(const Scene* scene, gr::Shader *shader);
};


