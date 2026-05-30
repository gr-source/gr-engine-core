#include "gr-base/system/transformsystem.hpp"

#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/view.hpp"

void TransformSystem::Update(Scene* scene)
{
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
}

