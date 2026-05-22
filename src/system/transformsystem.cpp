#include "gr-base/system/transformsystem.hpp"

#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/view.hpp"

void TransformSystem::Update(World* world)
{
    Signature requiredMask = Type::GetMask<Transform>();

    auto array = world->GetRawArcheType();

    for (size_t i=0;i<world->GetArcheTypeCount();i++)
    {
        ArchetypeStorage& archetype = array[i];

        if ((archetype.m_mask & requiredMask) == requiredMask)
        {
            auto transforms = archetype.GetRaw<Transform>();
            auto entities = archetype.GetEntities();

            for (ComponentID i=0;i<archetype.Count();i++)
            {
                auto& transform = transforms[i];
                auto& entity = entities[i];

                if (!transform.dirty)
                    continue;

                transform.localMatrix = Math::CreateTRS(transform.position, transform.rotation, transform.scale);

                transform.worldMatrix = transform.localMatrix;

                auto rel = world->GetComponent<Relationship>(entity);
                if (rel != nullptr && rel->parent != INVALID_ENTITY)
                {
                    Transform* parentTransform = world->GetComponent<Transform>(rel->parent);

                    transform.worldMatrix = parentTransform->worldMatrix * transform.localMatrix;
                }

                transform.dirty = false;
            }
        }
    }
}

