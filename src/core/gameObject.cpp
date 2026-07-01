#include "gr-base/core/gameObject.hpp"

#include "gr-base/core/scene.hpp"

bool GameObject::IsAlive() const
{
    return scene->GetWorldDynamic()->IsAlive(entity);
}

void* GameObject::AddComponent(TypeID typeID, const void* src)
{
    return scene->GetWorldDynamic()->AddComponent(entity, typeID, src);
}

void* GameObject::GetComponent(TypeID typeID) const
{
    return scene->GetWorldDynamic()->GetComponent(entity, typeID);
}

void* GameObject::GetOrAddComponent(TypeID typeID)
{
    return scene->GetWorldDynamic()->GetOrAddComponent(entity, typeID);
}

bool GameObject::HasComponent(Signature mask) const
{
    return scene->GetWorldDynamic()->HasComponent(entity, mask);
}

void GameObject::DelComponent(TypeID typeID)
{
    scene->GetWorldDynamic()->DelComponent(entity, typeID);
}



