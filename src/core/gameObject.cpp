#include "gr-base/core/gameObject.hpp"

#include "gr-base/ecs/world.hpp"

#include "gr-base/core/scene.hpp"

bool GameObject::IsAlive() const
{
    return scene->GetWorld()->IsAlive(entity);
}

void* GameObject::AddComponent(TypeID typeID, const void* src)
{
    scene->GetWorld()->AddComponent(entity, typeID, src);

    return GetComponent(typeID);
}

void* GameObject::GetComponent(TypeID typeID) const
{
    return scene->GetWorld()->GetComponent(entity, typeID);
}

void* GameObject::GetOrAddComponent(TypeID typeID)
{
    return scene->GetWorld()->GetOrAddComponent(entity, typeID);
}

bool GameObject::HasComponent(Signature mask) const
{
    return scene->GetWorld()->HasComponent(entity, mask);
}

void GameObject::DelComponent(TypeID typeID)
{
    scene->GetWorld()->DelComponent(entity, typeID);
}



