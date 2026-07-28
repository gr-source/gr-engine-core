#include "gr-base/system/renderSystem.hpp"

#include "gr-base/core/renderer.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/world.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/light.hpp"

#include "gr-base/spatial/bvhtree.hpp"

void RenderSystem::Update(Scene* scene)
{
    Entity cameraEntity = scene->GetCamera();
    if (cameraEntity == INVALID_ENTITY)
        return;

    auto world = scene->GetWorldDynamic();
    auto bvh = scene->GetBVHtree();

    UpdateBVH(scene);

    std::vector<Entity> entities;
    bvh->queryFrustumAABB(entities);

    for (auto& entity : entities)
    {
        auto transformWorld = world->GetComponent<TransformWorld>(entity);
        auto meshFilter = world->GetComponent<MeshFilter>(entity);
        auto meshRender = world->GetComponent<MeshRender>(entity);

        if (meshRender == nullptr)
            continue;

        auto& mesh = meshFilter->mesh;
        for (size_t j=0;j<meshRender->numMaterials;j++)
        {
            auto& material = meshRender->materials[j];

            Renderer::Submit(
                        material,
                        mesh,
                        static_cast<uint32_t>(j),
                        &transformWorld->worldMatrix
                    );
        }
    }
}

void RenderSystem::UpdateBVH(Scene* scene)
{
    auto bvh = scene->GetBVHtree();

    auto world = scene->GetWorldDynamic();

    world->Query<TransformWorld, RenderBounds, TransformDirty, RenderCache>(
            [&](
                size_t count,
                Entity* entities,
                TransformWorld* transformWorlds,
                RenderBounds* bounds,
                TransformDirty* dirty,
                RenderCache* cache)
            {
                for (size_t i=0;i<count;i++)
                {
                    if (dirty[i].version == cache[i].version)
                        continue;

                    auto& world = transformWorlds[i];

                    AABB local = bounds[i].local;
                    local.min = world.worldMatrix * local.min;
                    local.max = world.worldMatrix * local.max;

                    bvh->sync(entities[i], local);

                    cache[i].version = dirty[i].version;
                }
            }
        );
}



