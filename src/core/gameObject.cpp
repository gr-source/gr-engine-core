#include "gr-base/core/gameObject.hpp"

#include "gr-base/script/name.hpp"

std::string GameObject::GetName() const
{
    Name* name = GetComponent<Name>();
    if (name != nullptr)
        return name->value;

    EntityID entityID = handle_get_first(entity);

    return "Entity " + std::to_string(entityID);
}

void* GameObject::AddComponentByID(TypeID type)
{
    return world->AddComponentByID(entity, type);
}

void* GameObject::GetComponentByID(TypeID type) const
{
    return world->GetComponentByID(entity, type);
}

bool GameObject::HasComponentByID(TypeID type) const
{
    assert(world != nullptr);

    Signature mask = (1ull << type);

    return (world->GetMask(entity) & mask) == mask;
}

std::vector<TypeID> GameObject::GetAllComponentIDs() const
{
    return world->GetAllComponentType(entity);
}

