#include "gr-base/system/renderSystem.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/renderer.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/world.hpp"

#include "gr-base/scopeTimer.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/spatial/bvhtree.hpp"
#include <iostream>

void RenderSystem::Update()
{
    SyncBVHBounds();
}

void RenderSystem::Render()
{
    scene->bvhtree.queryFrustumAABB(entities);

    ScopeTimer scope;
    scope.start();

    for (auto& entity : entities)
    {
        auto&& [transform, meshFilter, meshRender] = scene->world.GetComponents<TransformWorld, MeshFilter, MeshRender>(entity);

        for (size_t j=0;j<meshRender->numMaterials;j++)
        {
            auto& material = meshRender->materials[j];

            Renderer::Submit(
                    material,
                    meshFilter->mesh,
                    static_cast<uint32_t>(j),
                    &transform->world
                );
        }
    }

    double ms = scope.end();

    std::cout << entities.size() << ": " << ms << std::endl;

    entities.clear();
}

#include "gr-base/ecs/view.hpp"

void RenderSystem::SyncBVHBounds()
{
    auto view =
        scene->world.view<TransformWorld, RenderBounds>()
        .Without<Static>();

    view.Each([&](
                size_t count,
                Entity*,
                TransformWorld* transforms,
                RenderBounds* renderBounds
            )
        {
            for (size_t i=0;i<count;i++)
            {
                auto& transform = transforms[i];
                auto& bound = renderBounds[i];

                const auto& local = bound.localBounds;

                Vector3 localCenter = local.Center();
                Vector3 localExtend = local.Extents();

                Vector3 worldCenter =
                    Math::transform_point(transform.world, localCenter);

                Vector3 worldExtend;
                worldExtend.x =
                    abs(transform.world[0][0]) * localExtend.x +
                    abs(transform.world[0][1]) * localExtend.y +
                    abs(transform.world[0][2]) * localExtend.z;

                worldExtend.y =
                    abs(transform.world[1][0]) * localExtend.x +
                    abs(transform.world[1][1]) * localExtend.y +
                    abs(transform.world[1][2]) * localExtend.z;

                worldExtend.z =
                    abs(transform.world[2][0]) * localExtend.x +
                    abs(transform.world[2][1]) * localExtend.y +
                    abs(transform.world[2][2]) * localExtend.z;

                AABB bounds;
                bounds.min = worldCenter - worldExtend;
                bounds.max = worldCenter + worldExtend;

                scene->bvhtree.sync(bound.id, bounds);
            }
        });


    /*
    if (!scene->Changed<TransformWorld>(entities))
        return;

    Vector3 corners[8];

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

            const auto& local = renderBounds->localBounds;

            corners[0] = { local.min.x, local.min.y, local.min.z };
            corners[1] = { local.min.x, local.min.y, local.max.z };
            corners[2] = { local.min.x, local.max.y, local.min.z };
            corners[3] = { local.min.x, local.max.y, local.max.z };
            corners[4] = { local.max.x, local.min.y, local.min.z };
            corners[5] = { local.max.x, local.min.y, local.max.z };
            corners[6] = { local.max.x, local.max.y, local.min.z };
            corners[7] = { local.max.x, local.max.y, local.max.z };

            Vector3 worldCorner = Math::transform_point(transformWorld->world, corners[0]);
            Vector3 worldMin = worldCorner;
            Vector3 worldMax = worldCorner;

            for (int i = 1; i < 8; ++i)
            {
                worldCorner = Math::transform_point(transformWorld->world, corners[i]);
                
                if (worldCorner.x < worldMin.x) worldMin.x = worldCorner.x;
                if (worldCorner.y < worldMin.y) worldMin.y = worldCorner.y;
                if (worldCorner.z < worldMin.z) worldMin.z = worldCorner.z;

                if (worldCorner.x > worldMax.x) worldMax.x = worldCorner.x;
                if (worldCorner.y > worldMax.y) worldMax.y = worldCorner.y;
                if (worldCorner.z > worldMax.z) worldMax.z = worldCorner.z;
            }

            AABB bounds;
            bounds.min = worldMin;
            bounds.max = worldMax;

            scene->bvhtree.sync(renderBounds->id, bounds);
        }
    }

    entities.clear();
    */
}



