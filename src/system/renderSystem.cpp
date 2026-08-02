#include "gr-base/system/renderSystem.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/renderer.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/world.hpp"

#include "gr-base/scopeTimer.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/light.hpp"

#include "gr-base/spatial/bvhtree.hpp"

void RenderSystem::Update()
{
    Entity cameraEntity = scene->GetCamera();
    if (cameraEntity == INVALID_ENTITY)
        return;

    auto world = scene->GetWorldDynamic();

    SyncBVHBounds();

    auto bvh = scene->GetBVHtree();

    bvh->queryFrustumAABB(entities);

    for (auto& entity : entities)
    {
        auto transform = world->GetComponent<TransformWorld>(entity);
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
                        &transform->world
                    );
        }
    }

    entities.clear();
}

#include <iostream>

void RenderSystem::SyncBVHBounds()
{
    if (!scene->Changed<TransformWorld>(entities))
        return;

    auto bvh = scene->GetBVHtree();

    for (auto& entity : entities)
    {
        GameObject gameObject(scene, entity);
        if (!gameObject.IsAlive())
            continue;

        if (gameObject.HasComponent<RenderBounds>())
        {
            auto renderBounds = gameObject.GetComponent<RenderBounds>();
            if (renderBounds->id == UINT32_MAX)
                continue;

            auto transformWorld = gameObject.GetComponent<TransformWorld>();

            AABB bounds = renderBounds->localBounds;
            bounds.min = transformWorld->world * bounds.min;
            bounds.max = transformWorld->world * bounds.max;

            bvh->sync(renderBounds->id, bounds);
        }
    }

    entities.clear();
}



