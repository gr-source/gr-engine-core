#include "gr-base/core/gameObject.hpp"

#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/ecs.hpp"

bool GameObject::IsAlive() const
{
    return scene->GetECS()->IsAlive(entity);
}

void* GameObject::AddComponent(TypeID typeID, const void* src)
{
    scene->GetECS()->AddComponent(entity, typeID, src);

    return GetComponent(typeID);
}

void* GameObject::GetComponent(TypeID typeID) const
{
    return scene->GetECS()->GetComponent(entity, typeID);
}

void* GameObject::GetOrAddComponent(TypeID typeID)
{
    return scene->GetECS()->GetOrAddComponent(entity, typeID);
}


void GameObject::DelComponent(TypeID typeID)
{
    scene->GetECS()->DelComponent(entity, typeID);
}



