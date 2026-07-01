#include "gr-base/system/transformsystem.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/world.hpp"

#include "gr-base/spatial/bvhtree.hpp"

void TransformSystem::Update(Scene* scene)
{
    BVHtree* bvh = scene->GetBVHtree();
    auto world = &scene->GetWorldDynamic();

    auto entities = scene->Changed<Transform>();
    for (auto& entity : entities)
    {
        auto transform = world->GetComponent<Transform>(entity);
        if (transform == nullptr)
            continue;

        transform->localMatrix = Math::CreateTRS(transform->position, transform->rotation, transform->scale);

        transform->worldMatrix = transform->localMatrix;

        auto rel = world->GetComponent<Relationship>(entity);
        if (rel != nullptr && rel->parent != INVALID_ENTITY)
        {
            Transform* parentTransform = world->GetComponent<Transform>(rel->parent);
            if (parentTransform != nullptr)
                transform->worldMatrix = parentTransform->worldMatrix * transform->localMatrix;
        }

        if (world->HasComponent<Transform, MeshFilter, MeshRender>(entity))
        {
            auto meshFilter = world->GetComponent<MeshFilter>(entity);
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

