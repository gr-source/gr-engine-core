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
#include <iomanip>
#include <iostream>

void RenderSystem::Update()
{
    auto world = scene->world;

    SyncBVHBounds();

    auto bvh = scene->GetBVHtree();

    {
        // ScopeTimer scope;
        // scope.start();
        bvh->queryFrustumAABB(entities);
        // double ms = scope.end();
        //
        // std::cout << std::fixed << std::setprecision(3) << "queryFrustumAABB: " << ms << std::endl;
    }

    for (auto& entity : entities)
    {
        if (!world->HasComponent<TransformWorld, MeshFilter, MeshRender>(entity))
            continue;

        auto&& [transform, meshFilter, meshRender] = world->GetComponents<TransformWorld, MeshFilter, MeshRender>(entity);

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

Vector3 MultiplyPoint(const Matrix4x4& lhs, const Vector3& rhs)
{
    Vector3 res =
    {
        lhs[0].x * rhs.x + lhs[1].x * rhs.y + lhs[2].x * rhs.z + lhs[3].x,
        lhs[0].y * rhs.x + lhs[1].y * rhs.y + lhs[2].y * rhs.z + lhs[3].y,
        lhs[0].z * rhs.x + lhs[1].z * rhs.y + lhs[2].z * rhs.z + lhs[3].z
    };

    float magSq = lhs[0].w * rhs.x + lhs[1].w * rhs.y + lhs[2].w * rhs.z + lhs[3].w;
    if (magSq > 1e-10f)
    {
        f32 invMag = 1.0f / std::sqrt(magSq);
        return res * invMag;
    }

    return type_traits<Vector3>::zero;
}

void RenderSystem::SyncBVHBounds()
{
    if (!scene->Changed<TransformWorld>(entities))
        return;

    Vector3 corners[8];

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

            const auto& local = renderBounds->localBounds;

            corners[0] = { local.min.x, local.min.y, local.min.z };
            corners[1] = { local.min.x, local.min.y, local.max.z };
            corners[2] = { local.min.x, local.max.y, local.min.z };
            corners[3] = { local.min.x, local.max.y, local.max.z };
            corners[4] = { local.max.x, local.min.y, local.min.z };
            corners[5] = { local.max.x, local.min.y, local.max.z };
            corners[6] = { local.max.x, local.max.y, local.min.z };
            corners[7] = { local.max.x, local.max.y, local.max.z };

            Vector3 worldCorner = MultiplyPoint(transformWorld->world, corners[0]);
            Vector3 worldMin = worldCorner;
            Vector3 worldMax = worldCorner;

            for (int i = 1; i < 8; ++i)
            {
                worldCorner = MultiplyPoint(transformWorld->world, corners[i]);
                
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

            bvh->sync(renderBounds->id, bounds);
        }
    }

    entities.clear();
}



