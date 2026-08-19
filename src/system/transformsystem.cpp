#include "gr-base/system/transformsystem.hpp"

#include "gr-base/core/gameObject.hpp"

#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/view.hpp"

void TransformSystem::Update()
{
    auto view =
        scene->world.view<TransformLocal, TransformWorld>()
        .Without<Static>();

    view.Each(
        [&](size_t count,
           Entity* entities,
           TransformLocal* local,
           TransformWorld* world)
        {
            for (size_t i = 0; i < count; ++i)
            {
                world[i].local =
                    Math::CreateTRS(
                        local[i].position,
                        local[i].rotation,
                        local[i].scale
                    );

                world[i].world = world[i].local;
            }
        });

    /*
    if (!scene->Changed<TransformWorld>(entities))
        return;

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
    */
}

bool TransformSystem::SyncTransform(const GameObject& gameObject, const Matrix4x4& parentMatrix) const
{
    auto& transform = gameObject.GetComponent<TransformWorld>();
    transform.world = parentMatrix * transform.local;

    auto rel = gameObject.TryGetComponent<Relationship>();
    if (rel == nullptr)
        return false;

    Entity firstChild = rel->firstChild;
    while (firstChild != INVALID_ENTITY)
    {
        GameObject child(scene, firstChild);
        if (!SyncTransform(child, transform.world))
            break;

        rel = child.TryGetComponent<Relationship>();
        firstChild = rel->nextSibling;
    }

    return true;
}

