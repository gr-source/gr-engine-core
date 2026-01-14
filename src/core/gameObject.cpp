#include "gr-base/core/gameObject.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/managers/gameobjectmanager.hpp"

#include "gr-base/script/transform.hpp"

GameObject* GameObject::Instantiate(GameObject *other)
{
    return GameObjectManager::Instantiate(other);
}

GameObject* GameObject::Create(const std::string &name)
{
    return GameObjectManager::Get(GameObjectManager::Create(name));
}

void GameObject::DestroyImediate(GameObject* gameObject)
{
    GameObjectManager::DestroyImediate(gameObject->getCacheID());
}

void GameObject::Destroy(GameObject* gameObject)
{
    GameObjectManager::Destroy(gameObject->getCacheID());
}

// public
GameObject::GameObject(GameObjectID id) : m_id(id), m_transform(nullptr), m_name("noname")
{
    m_transform = addComponent<Transform>();
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
    return m_transform;
}

GameObjectID GameObject::getCacheID() const
{
    return m_id;
}

void GameObject::SetState(bool enable)
{}

void GameObject::Copy(const GameObject* other)
{
    for (auto& hash : other->GetHashes())
    {
        Behavior* behavior = getComponent(hash);
        if (behavior == nullptr)
        {
            behavior = addComponent(hash);

            ComponentManager::Reinstance(hash, behavior, other->getComponent(hash));
            behavior->entity = this;
        }
    }

    auto targetTransform = other->getTransform();
    
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

        auto clonedObject = GameObject::Create(childGameObject->getName());
        if (!clonedObject)
        {
            Console::LogError("[ Entity ] Failed to create child object %s->%s.", other->getName().c_str(), childGameObject->getName().c_str());
            continue;
        }

        clonedObject->Copy(childGameObject);

        auto clonedTransform = clonedObject->getTransform();
        clonedTransform->setParent(localTransform);
    }
}


