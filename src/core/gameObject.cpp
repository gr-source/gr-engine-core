#include "gr-base/core/gameObject.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/default.hpp"
#include "gr-base/core/file/stream.hpp"

#include "gr-base/ecs/behaviorStorage.hpp"
#include "gr-base/ecs/default.hpp"
#include "gr-base/managers/gameobjectmanager.hpp"

#include "gr-base/script/transform.hpp"

GameObjectHandle GameObject::Instantiate(GameObjectHandle other)
{
    return GameObjectManager::Instantiate(other);
}

GameObjectHandle GameObject::Create(const std::string &name)
{
    return GameObjectManager::Create(name);
}

GameObject* GameObject::Get(GameObjectHandle handle)
{
    return GameObjectManager::Get(handle);
}

void GameObject::DestroyImediate(GameObjectHandle gameObjectID)
{
    GameObjectManager::DestroyImediate(gameObjectID);
}

void GameObject::Destroy(GameObjectHandle gameObjectID)
{
    GameObjectManager::Destroy(gameObjectID);
}

// public
GameObject::GameObject(GameObjectHandle id) : Entity(id), m_name("noname"), m_scene(nullptr), m_isStatic(false), m_transform(HANDLE_INVALID)
{
    addComponent<Transform>();

    m_transform = GetID<Transform>();
}

GameObject::GameObject(GameObject&& other) noexcept : Entity(std::move(other))
{
    // GameObject
    m_name = other.m_name;
    m_scene = other.m_scene;
    m_isStatic = other.m_isStatic;

    m_transform = other.m_transform;
}

GameObject::~GameObject()
{
    for (auto &hash : GetHashes())
        BehaviorStorage::Deallocate(hash, GetID(hash));
}

Transform* GameObject::GetTransform() const
{
    return BehaviorStorage::GetComponent<Transform>(m_transform);
}

void GameObject::Copy(GameObjectHandle otherID)
{
    auto otherGO = GameObject::Get(otherID);
    if (otherGO == nullptr)
        return;

    m_isStatic = otherGO->m_isStatic;

    auto otherTrans = otherGO->GetTransform();

    ComponentHandle myTransformID = m_transform;

    auto myTrans = GetTransform();
    myTrans->setLocalPosition(otherTrans->getLocalPosition());
    myTrans->setLocalRotation(otherTrans->getLocalRotation());
    myTrans->setLocalScale(otherTrans->getLocalScale());

    for (auto& transformID : otherTrans->getChildren())
    {
        auto childTransform = BehaviorStorage::GetComponent<Transform>(transformID);

        auto childGO = childTransform->GetGameObject();
        GameObjectHandle childID = childGO->GetCacheID();

        GameObjectHandle newChildID = GameObject::Create(childGO->GetName());

        auto newChildGO = GameObject::Get(newChildID);
        newChildGO->GetTransform()->setParent(BehaviorStorage::GetComponent<Transform>(myTransformID));

        newChildGO->Copy(childID);
    }

    otherGO = GameObject::Get(otherID);
    for (auto& hash : otherGO->GetHashes())
    {
        if (hash == GetHashByType<Transform>(nullptr))
            continue;

        Behavior* myComp = getComponent(hash);
        if (myComp == nullptr)
            myComp = addComponent(hash, otherGO->GetID(hash));
    }
}

GameObjectHandle GameObject::FindChildByName(const std::string& name)
{
    if (m_name == name)
        return GetCacheID();

    for (auto &transformID : GetTransform()->getChildren())
    {
        auto transform = BehaviorStorage::GetComponent<Transform>(transformID);

        GameObjectHandle found = transform->GetGameObject()->FindChildByName(name);
        if (found != HANDLE_INVALID)
            return found;
    }

    return HANDLE_INVALID;
}




