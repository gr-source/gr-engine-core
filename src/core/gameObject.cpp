#include "gr-base/core/gameObject.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/default.hpp"
#include "gr-base/core/file/stream.hpp"

#include "gr-base/ecs/behaviorStorage.hpp"
#include "gr-base/ecs/default.hpp"
#include "gr-base/managers/gameobjectmanager.hpp"

#include "gr-base/script/transform.hpp"

GameObjectHandle GameObject::Instantiate(GameObject *other)
{
    return GameObjectManager::Instantiate(other->getCacheID());
}

GameObjectHandle GameObject::Create(const std::string &name)
{
    return GameObjectManager::Create(name);
}

GameObject* GameObject::Get(GameObjectHandle handle)
{
    return GameObjectManager::Get(handle);
}

void GameObject::DestroyImediate(GameObject* gameObject)
{
    GameObjectManager::DestroyImediate(gameObject->getCacheID());
}

void GameObject::DestroyImediate(GameObjectHandle gameObjectID)
{
    GameObjectManager::DestroyImediate(gameObjectID);
}

void GameObject::Destroy(GameObject* gameObject)
{
    GameObjectManager::Destroy(gameObject->getCacheID());
}

void GameObject::Destroy(GameObjectHandle gameObjectID)
{
    GameObjectManager::Destroy(gameObjectID);
}

// public
GameObject::GameObject(GameObjectHandle id) : Entity(id), m_name("noname"), m_scene(nullptr), m_isStatic(false)
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

Transform* GameObject::GetTransform() const
{
    return getComponent<Transform>();
}

void GameObject::SetState(bool enable)
{}

void GameObject::Copy(GameObjectHandle otherID)
{
    auto otherGO = GameObject::Get(otherID);
    if (!otherGO)
        return;

    m_isStatic = otherGO->m_isStatic;

    auto otherTrans = otherGO->GetTransform();

    auto myTrans = this->GetTransform();
    myTrans->setLocalPosition(otherTrans->getLocalPosition());
    myTrans->setLocalRotation(otherTrans->getLocalRotation());
    myTrans->setLocalScale(otherTrans->getLocalScale());

    for (auto& hash : otherGO->GetHashes())
    {
        if (hash == GetHashByType<Transform>(nullptr))
            continue;

        Behavior* myComp = getComponent(hash);
        if (myComp == nullptr)
            myComp = addComponent(hash, otherGO->GetID(hash));

        // BehaviorStorage::Reinstance(hash, myComp, otherGO->getComponent(hash));
        // myComp->entityID = this->getCacheID();
    }

    for (const auto& childTrans : otherTrans->getChildren())
    {
        auto c = BehaviorStorage::GetComponent<Transform>(childTrans);

        auto childGO = c->GetGameObject();
        
        GameObjectHandle newChildID = GameObject::Create(childGO->getName());
        auto newChildGO = GameObject::Get(newChildID);
        
        newChildGO->Copy(childGO->getCacheID());

        newChildGO->GetTransform()->setParent(this->GetTransform());
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

void GameObject::SetScene(Scene* scene)
{
    m_scene = scene;
}

Scene* GameObject::GetScene() const
{
    return m_scene;
}

void GameObject::SetIsStatic(bool b)
{
    m_isStatic = b;
}

bool GameObject::GetIsStatic() const
{
    return m_isStatic;
}


GameObjectHandle GameObject::FindChildByName(const std::string& name)
{
    if (m_name == name)
        return getCacheID();

    for (auto &transformID : GetTransform()->getChildren())
    {
        auto transform = BehaviorStorage::GetComponent<Transform>(transformID);

        GameObjectHandle found = transform->GetGameObject()->FindChildByName(name);
        if (found != HANDLE_INVALID)
            return found;
    }

    return HANDLE_INVALID;
}

// private
void GameObject::Serialize(GameObjectHandle id, Stream* stream)
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

void GameObject::Deserialize(Stream* stream, GameObjectHandle id, ComponentID parent)
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


