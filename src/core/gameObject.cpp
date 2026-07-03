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

void* GameObject::AddComponentByName(const std::string& name)
{
    TypeID type = ComponentRegistry::GetTypeIDByName(name.c_str());
    assert(type != INVALID_TYPE);

    return AddComponentByID(type);
}

void* GameObject::AddComponentByID(TypeID type)
{
    return world->AddComponentByID(entity, type);
}

void* GameObject::GetComponentByName(const std::string& name) const
{
    TypeID type = ComponentRegistry::GetTypeIDByName(name.c_str());
    assert(type != INVALID_TYPE);

    return GetComponentByID(type);
}

void* GameObject::GetComponentByID(TypeID type) const
{
    return world->GetComponentByID(entity, type);
}

bool GameObject::HasComponentByName(const std::string& name) const
{
    TypeID type = ComponentRegistry::GetTypeIDByName(name.c_str());
    assert(type != INVALID_TYPE);

    return HasComponentByID(type);
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

