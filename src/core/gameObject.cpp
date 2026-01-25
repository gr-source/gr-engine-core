#include "gr-base/core/gameObject.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/default.hpp"
#include "gr-base/core/file/stream.hpp"

#include "gr-base/ecs/behaviorStorage.hpp"
#include "gr-base/ecs/default.hpp"
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
        BehaviorStorage::Deallocate(hash, GetID(hash));
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

void GameObject::Copy(GameObjectID otherID)
{
    auto otherGO = GameObject::Get(otherID);
    if (!otherGO)
        return;

    for (auto& hash : otherGO->GetHashes())
    {
        if (hash == GetHashByType<Transform>(nullptr))
            continue;

        Behavior* sourceComp = otherGO->getComponent(hash);

        Behavior* myComp = getComponent(hash);
        if (!myComp)
            myComp = addComponent(hash, otherGO->GetID(hash));

        BehaviorStorage::Reinstance(hash, myComp, sourceComp);
        myComp->entityID = this->getCacheID();
    }

    auto otherTrans = otherGO->getTransform();

    auto myTrans = this->getTransform();
    myTrans->setLocalPosition(otherTrans->getLocalPosition());
    myTrans->setLocalRotation(otherTrans->getLocalRotation());
    myTrans->setLocalScale(otherTrans->getLocalScale());

    for (const auto& childTrans : otherTrans->getChildren())
    {
        auto c = BehaviorStorage::GetComponent<Transform>(childTrans);

        auto childGO = c->GetGameObject();
        
        GameObjectID newChildID = GameObject::Create(childGO->getName());
        auto newChildGO = GameObject::Get(newChildID);
        
        newChildGO->Copy(childGO->getCacheID());

        newChildGO->getTransform()->setParent(this->getTransform());
    }
}

void GameObject::Serialize(Stream* stream) const
{
    Serialize(getCacheID(), stream);
}

void GameObject::Deserialize(Stream* stream)
{
    Deserialize(stream, getCacheID(), INVALID_COMPONENT_ID);
}

// private
void GameObject::Serialize(GameObjectID id, Stream* stream)
{
    /*
    auto gameObject = GameObject::Get(id);
    if (!stream->write(gameObject->m_name))
        return;

    auto hashes = gameObject->GetHashes();

    uint64_t hashcount = static_cast<uint64_t>(hashes.size());
    if (!stream->write(hashcount))
        return;

    for (auto& hash : hashes)
    {
        if (!stream->write(hash))
            return;

        auto behavior = gameObject->getComponent(hash);

        uint64_t bufferlength = 0;
        {
            Stream temp;
            behavior->Serialize(&temp);
            bufferlength = temp.getDataSize();
            temp.close();
        }

        if (!stream->write(bufferlength))
            return;

        behavior->Serialize(stream);
    }

    auto transform = gameObject->getTransform();

    auto children = transform->getChildren();

    uint64_t childCount = static_cast<uint64_t>(children.size());
    if (!stream->write(childCount))
        return;

    for (auto child : children)
        Serialize(child->entityID, stream);
    */
}

void GameObject::Deserialize(Stream* stream, GameObjectID id, ComponentID parent)
{
    /*
    auto gameObject = GameObject::Get(id);
    if (!stream->read(gameObject->m_name))
        return;

    auto transform = gameObject->getTransform();

    if (parent != INVALID_COMPONENT_ID)
        transform->setParent(BehaviorStorage::GetComponent<Transform>(parent));

    uint64_t hashcount = 0;
    if (!stream->read(hashcount))
        return;

    for (uint64_t i=0;i<hashcount;i++)
    {
        uint64_t hash = INVALID_COMPONENT_ID;
        if (!stream->read(hash) || hash == INVALID_COMPONENT_ID)
            return;

        uint64_t bufferlength = 0;
        if (!stream->read(bufferlength))
            return;

        auto next = stream->tellg() + static_cast<std::streampos>(bufferlength);

        Behavior* behavior = gameObject->getComponent(hash);
        if (behavior == nullptr)
            behavior = gameObject->addComponent(hash);

        if (behavior == nullptr)
        {
            stream->seekg(next, std::ios_base::beg);
            continue;
        }

        behavior->Deserialize(stream);
    }

    uint64_t childCount = 0;
    if (!stream->read(childCount))
        return;

    for (uint64_t i=0;i<childCount;i++)
        Deserialize(stream, GameObject::Create("unknow"), gameObject->GetID<Transform>());
    */
}


