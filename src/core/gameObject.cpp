#include "gr-base/core/gameObject.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/managers/gameobjectmanager.hpp"

#include "gr-base/script/transform.hpp"

GameObjectID GameObject::Instantiate(GameObject *other)
{
    return GameObjectManager::Instantiate(other);
}

GameObjectID GameObject::Create(const std::string &name)
{
    return GameObjectManager::Create(name);
}

GameObject* GameObject::Get(GameObjectID handle)
{
    return GameObjectManager::Get(handle);
}

void GameObject::DestroyImediate(GameObject* gameObject)
{
    GameObjectManager::DestroyImediate(gameObject->getCacheID());
}

void GameObject::DestroyImediate(GameObjectID gameObjectID)
{
    GameObjectManager::DestroyImediate(gameObjectID);
}

void GameObject::Destroy(GameObject* gameObject)
{
    GameObjectManager::Destroy(gameObject->getCacheID());
}

void GameObject::Destroy(GameObjectID gameObjectID)
{
    GameObjectManager::Destroy(gameObjectID);
}

// public
GameObject::GameObject(GameObjectID id) : Entity(id), m_name("noname")
{
    addComponent<Transform>();
}

GameObject::~GameObject()
{
    for (auto &hash : GetHashes())
    {
        ComponentManager::Deallocate(hash, GetID(hash));
    }
}

std::string GameObject::getName() const
{
    return m_name;
}

void GameObject::setName(const std::string& name)
{
    m_name = name;
}

Transform* GameObject::getTransform() const
{
    return getComponent<Transform>();
}

void GameObject::SetState(bool enable)
{}

void GameObject::Copy(GameObjectID other)
{
    {
        auto gameObject = GameObject::Get(other);
        for (auto& hash : gameObject->GetHashes())
        {
            Behavior* behavior = getComponent(hash);
            if (behavior == nullptr)
                behavior = addComponent(hash, gameObject->GetID(hash));
            /*
            else
            {
                ComponentManager::Reinstance(hash, behavior, gameObject->getComponent(hash));
                behavior->entityID = getCacheID();
            }
            */
        }
    }

    auto targetTransform = GameObject::Get(other)->getTransform();
    
    auto localTransform = getTransform();
    localTransform->setLocalScale(targetTransform->getLocalScale());
    localTransform->setLocalRotation(targetTransform->getLocalRotation());
    localTransform->setLocalPosition(targetTransform->getLocalPosition());

    if (targetTransform->hasParent())
    {
        localTransform->setParent(targetTransform->getParent());
    }

    for (const auto& child : targetTransform->getChildren())
    {
        auto childGameObject = child->GetGameObject();

        auto childID = GameObject::Create(childGameObject->getName());

        auto cloneObj = GameObjectManager::Get(childID);
        cloneObj->Copy(childID);

        auto clonedTransform = cloneObj->getTransform();
        clonedTransform->setParent(localTransform);
    }
}


