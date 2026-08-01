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

void TransformSystem::Update()
{
    BVHtree* bvh = scene->GetBVHtree();

    auto entities = scene->Changed<Transform>();
    for (auto& entity : entities)
    {
        GameObject gameObject(scene, entity);
        if (!gameObject.HasComponent<Transform>())
            continue;

        auto transform = gameObject.GetComponent<Transform>();
        
        Matrix4x4 parentMatrix = type_traits<Matrix4x4>::identity;

        if (gameObject.HasComponent<Relationship>())
        {
            auto rel = gameObject.GetComponent<Relationship>();
            if (rel->parent != INVALID_ENTITY)
            {
                GameObject parent(scene, rel->parent);
                if (parent.HasComponent<Transform>())
                {
                    auto parentTransform = parent.GetComponent<Transform>();
                    parentMatrix = parentTransform->worldMatrix;
                }
            }
        }

        transform->localMatrix = Math::CreateTRS(transform->position, Math::normalize(transform->rotation), transform->scale);

        SyncTransform(gameObject, parentMatrix);
    }
}

bool TransformSystem::SyncTransform(const GameObject& gameObject, const Matrix4x4& parentMatrix) const
{
    auto bvh = scene->GetBVHtree();

    auto transform = gameObject.GetComponent<Transform>();

    transform->worldMatrix = parentMatrix * transform->localMatrix;

    if (gameObject.HasComponent<MeshFilter, MeshRender>())
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

                bvh->sync(gameObject.entity, bounds);
            }
        }
    }

    auto rel = gameObject.GetComponent<Relationship>();
    if (rel == nullptr)
        return false;

    Entity firstChild = rel->firstChild;
    while (firstChild != INVALID_ENTITY)
    {
        GameObject child(scene, firstChild);
        if (!SyncTransform(child, transform->worldMatrix))
            break;

        rel = child.GetComponent<Relationship>();
        firstChild = rel->nextSibling;
    }

    return true;
}

