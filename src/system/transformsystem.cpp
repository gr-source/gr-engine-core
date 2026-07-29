#include "gr-base/system/transformsystem.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/scopeTimer.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/core/scene.hpp"

#include "gr-base/spatial/bvhtree.hpp"

void TransformSystem::Update(Scene* scene)
{
    BVHtree* bvh = scene->GetBVHtree();

    /*
    size_t updated = 0;

    {
        ScopeTimer scope("TransformSystem");
        scene->world->Query<TransformLocal, TransformWorld, TransformDirty, TransformCache>
            (
                [&](
                    size_t count,
                    Entity* entities,
                    TransformLocal* locals,
                    TransformWorld* worlds,
                    TransformDirty* dirty,
                    TransformCache* cache

                )
                {
                    for (size_t i=0;i<count;i++)
                    {
                        if (dirty[i].version == cache[i].version)
                            continue;

                        updated++;

                        // auto& local = locals[i];
                        // auto& world = worlds[i];
                        //
                        // world.localMatrix =
                        //     Math::CreateTRS(
                        //             local.position,
                        //             local.rotation,
                        //             local.scale
                        //         );
                        // world.worldMatrix = world.localMatrix;

                        cache[i].version = dirty[i].version;
                    }
                }
            );
    }

    std::cout << "Update: " << updated << std::endl;
    */

    auto entities = scene->Changed<Transform>();
    for (auto& entity : entities)
    {
        GameObject gameObject(scene, entity);
        if (!gameObject.HasComponent<Transform>())
            continue;

        auto transform = gameObject.GetComponent<Transform>();
        transform->localMatrix = Math::CreateTRS(transform->position, transform->rotation, transform->scale);
        transform->worldMatrix = transform->localMatrix;

        if (gameObject.HasComponent<Relationship>())
        {
            auto rel = gameObject.GetComponent<Relationship>();
            if (rel->parent != INVALID_ENTITY)
            {
                GameObject parent(scene, rel->parent);
                if (parent.HasComponent<Transform>())
                {
                    auto parentTransform = parent.GetComponent<Transform>();
                    transform->worldMatrix = parentTransform->worldMatrix * transform->localMatrix;
                }
            }
        }

        if (gameObject.HasComponent<Transform, MeshFilter, MeshRender>())
        {
            auto meshFilter = gameObject.GetComponent<MeshFilter>();
            if (meshFilter != nullptr)
            {
                auto mesh = AssetsManager::GetMesh(meshFilter->mesh);
                if (mesh != nullptr)
                {
                    AABB bounds = mesh->localBounds;
                    bounds.min = transform->worldMatrix * bounds.min;
                    bounds.max = transform->worldMatrix * bounds.max;

                    bvh->sync(entity, bounds);
                }
            }
        }
    }
}

