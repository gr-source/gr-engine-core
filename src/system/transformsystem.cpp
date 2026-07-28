#include "gr-base/system/transformsystem.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/core/scene.hpp"

#include "gr-base/spatial/bvhtree.hpp"

void TransformSystem::Update(Scene* scene)
{
    BVHtree* bvh = scene->GetBVHtree();

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

                    auto& local = locals[i];
                    auto& world = worlds[i];

                    world.localMatrix =
                        Math::CreateTRS(
                                local.position,
                                local.rotation,
                                local.scale
                            );
                    world.worldMatrix = world.localMatrix;

                    /*
                    if (gameObject.HasComponent<Relationship>())
                    {
                        auto rel = gameObject.GetComponent<Relationship>();
                        if (rel->parent != INVALID_ENTITY)
                        {
                            GameObject parent(scene, rel->parent);
                            if (parent.HasComponent<TransformWorld>())
                            {
                                auto parentTransform = parent.GetComponent<TransformWorld>();
                                world.worldMatrix = parentTransform->worldMatrix * world.localMatrix;
                            }
                        }
                    }

                    GameObject gameObject(scene, entities[i]);
                    if (gameObject.HasComponent<MeshFilter, MeshRender>())
                    {
                        auto meshFilter = gameObject.GetComponent<MeshFilter>();
                        if (meshFilter != nullptr)
                        {
                            auto mesh = AssetsManager::GetMesh(meshFilter->mesh);
                            if (mesh != nullptr)
                            {
                                AABB bounds = mesh->localBounds;
                                bounds.min = world.worldMatrix * bounds.min;
                                bounds.max = world.worldMatrix * bounds.max;

                                bvh->sync(entities[i], bounds);
                            }
                        }
                    }
                    */

                    cache[i].version = dirty[i].version;
                }
            }
        );

    /*
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
    */

    /*
    World* world = scene->GetWorld();

    Signature requiredMask = Type::GetMask<Transform>();

    auto array = world->GetRawArchetype();

    for (size_t i=0;i<world->GetArchetypeCount();i++)
    {
        ArchetypeStorage& arche = array[i];

        if ((arche.m_mask & requiredMask) == requiredMask)
        {
            auto transforms = arche.GetRaw<Transform>();
            auto entities = arche.slotToEntity.data();

            for (ComponentID i=0;i<arche.entityCount;i++)
            {
                auto& transform = transforms[i];

                if (!transform.dirty)
                    continue;

                auto& entity = entities[i];

                transform.localMatrix = Math::CreateTRS(transform.position, transform.rotation, transform.scale);

                transform.worldMatrix = transform.localMatrix;

                auto rel = world->GetComponent<Relationship>(entity);
                if (rel != nullptr && rel->parent != INVALID_ENTITY)
                {
                    Transform* parentTransform = world->GetComponent<Transform>(rel->parent);
                    if (parentTransform != nullptr)
                        transform.worldMatrix = parentTransform->worldMatrix * transform.localMatrix;
                }

                transform.dirty = false;
            }
        }
    }
    */
}

