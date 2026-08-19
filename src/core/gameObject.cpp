#include "gr-base/core/gameObject.hpp"

#include "gr-base/ecs/componentRegistry.hpp"

#include "gr-base/script/name.hpp"

GameObject::GameObject(Scene* scene, Entity entity) :
    scene(scene), entity(entity)
{}

bool GameObject::IsAlive() const
{
    return scene->world.IsAlive(entity);
}

std::string GameObject::GetName() const
{
    if (Name* name = TryGetComponent<Name>())
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
    return scene->world.AddComponentByID(entity, type);
}

void* GameObject::GetComponentByName(const std::string& name) const
{
    TypeID type = ComponentRegistry::GetTypeIDByName(name.c_str());
    assert(type != INVALID_TYPE);

    return GetComponentByID(type);
}

void* GameObject::GetComponentByID(TypeID type) const
{
    return scene->world.GetComponentByID(entity, type);
}

bool GameObject::HasComponentByName(const std::string& name) const
{
    TypeID type = ComponentRegistry::GetTypeIDByName(name.c_str());
    assert(type != INVALID_TYPE);

    return HasComponentByID(type);
}

bool GameObject::HasComponentByID(TypeID type) const
{
    Signature mask = (1ull << type);

    return (scene->world.GetMask(entity) & mask) == mask;
}

std::vector<TypeID> GameObject::GetAllComponentIDs() const
{
    return scene->world.GetAllComponentType(entity);
}

