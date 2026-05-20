#include "gr-base/system/transformsystem.hpp"

#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/view.hpp"
#include "gr-base/ecs/ecs.hpp"

void TransformSystem::Update(ECS* world)
{
    for (auto&& [e, transform] : world->Query<Transform>())
    {
        if (!transform->dirty)
            continue;

        transform->localMatrix = Math::CreateTRS(transform->position, transform->rotation, transform->scale);

        transform->worldMatrix = transform->localMatrix;

        auto rel = world->GetComponent<Relationship>(e);
        if (rel != nullptr && rel->parent != INVALID_ENTITY)
        {
            Transform* parentTransform = world->GetComponent<Transform>(rel->parent);

            transform->worldMatrix = parentTransform->worldMatrix * transform->localMatrix;
        }

        transform->dirty = false;
    }
}

