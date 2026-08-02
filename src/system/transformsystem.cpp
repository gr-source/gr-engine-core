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

#include <iostream>

void TransformSystem::Update()
{
    if (!scene->Changed<TransformWorld>(entities))
        return;

    BVHtree* bvh = scene->GetBVHtree();
    for (auto& entity : entities)
    {
        GameObject gameObject(scene, entity);
        if (!gameObject.HasComponent<TransformWorld>())
            continue;

        auto transform = gameObject.GetComponent<TransformLocal>();
        auto transformWorld = gameObject.GetComponent<TransformWorld>();
        
        Matrix4x4 parentMatrix = type_traits<Matrix4x4>::identity;

        if (gameObject.HasComponent<Relationship>())
        {
            auto rel = gameObject.GetComponent<Relationship>();
            if (rel->parent != INVALID_ENTITY)
            {
                GameObject parent(scene, rel->parent);
                if (parent.HasComponent<TransformWorld>())
                {
                    auto parentTransform = parent.GetComponent<TransformWorld>();
                    parentMatrix = parentTransform->world;
                }
            }
        }

        transformWorld->local = Math::CreateTRS(transform->position, Math::normalize(transform->rotation), transform->scale);

        SyncTransform(gameObject, parentMatrix);
    }

    entities.clear();
}

bool TransformSystem::SyncTransform(const GameObject& gameObject, const Matrix4x4& parentMatrix) const
{
    auto bvh = scene->GetBVHtree();

    auto transform = gameObject.GetComponent<TransformWorld>();
    transform->world = parentMatrix * transform->local;

    auto rel = gameObject.GetComponent<Relationship>();
    if (rel == nullptr)
        return false;

    Entity firstChild = rel->firstChild;
    while (firstChild != INVALID_ENTITY)
    {
        GameObject child(scene, firstChild);
        if (!SyncTransform(child, transform->world))
            break;

        rel = child.GetComponent<Relationship>();
        firstChild = rel->nextSibling;
    }

    return true;
}

