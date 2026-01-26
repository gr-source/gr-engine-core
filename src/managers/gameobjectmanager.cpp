#include "gr-base/managers/gameobjectmanager.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/default.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/behaviorStorage.hpp"
#include "gr-base/externals/array.h"
#include "gr-base/script/transform.hpp"

#include "gr-base/managers/sceneManager.hpp"

#include <string.h>

typedef struct GameObjectWrapper
{
    uint32_t slotID;
    GameObject gameObject;
} GameObjectWrapper;

typedef struct GameObjectSlot
{
    uint32_t denseIndex;
    uint32_t version;
    uint32_t next;
} GameObjectSlot;

GameObjectManager* GameObjectManager::s_GameObjectManager = nullptr;

/*
void GameObjectManager::Initialize()
{
    if (s_GameObjectManager == nullptr)
        s_GameObjectManager = new GameObjectManager();
}

void GameObjectManager::Shutdown()
{
    if (s_GameObjectManager != nullptr)
        delete s_GameObjectManager;
    s_GameObjectManager = nullptr;
}
*/

void GameObjectManager::SetInstance(GameObjectManager* gameobjectmanager)
{
    s_GameObjectManager = gameobjectmanager;
}

GameObjectID GameObjectManager::Instantiate(const GameObject *other)
{
    return s_GameObjectManager->instantiate(other->getCacheID());
}

GameObjectID GameObjectManager::Create(const std::string &name)
{
    return s_GameObjectManager->create(name);
}

GameObject* GameObjectManager::Get(GameObjectID id)
{
    return s_GameObjectManager->get(id);
}

void GameObjectManager::DestroyImediate(GameObjectID id)
{
    s_GameObjectManager->destroyImediate(id);
}

void GameObjectManager::Destroy(GameObjectID id)
{
    s_GameObjectManager->destroy(id);
}

size_t GameObjectManager::GetCount()
{
    return s_GameObjectManager->m_count;
}

// public
GameObjectManager::GameObjectManager() : m_gameObjectPool(nullptr), m_slot(nullptr), m_freeIDs(n32nill), m_count(0)
{
    m_gameObjectPool = array_create(1024, sizeof(GameObjectWrapper));
    m_slot = array_create(1024, sizeof(GameObjectSlot));

    GameObjectSlot* slots = (GameObjectSlot*)m_slot->vector;
    for (int i=0;i<m_slot->length;i++)
        slots[i] = {INVALID_COMPONENT_ID, 1, INVALID_COMPONENT_ID};

    addFreeID(0);
}

GameObjectManager::~GameObjectManager()
{
    array_free(m_gameObjectPool);
    array_free(m_slot);
}

// private
GameObjectID GameObjectManager::instantiate(GameObjectID other)
{
    auto handle = create(GameObject::Get(other)->getName());
    if (handle == GID_INVALID)
        return GID_INVALID;

    auto gameObject = get(handle);
    gameObject->Copy(other);

    auto scene = SceneManager::GetCurrent();
    if (scene != nullptr)
        scene->insert(handle);

    return handle;
}

GameObjectID GameObjectManager::create(const std::string &name)
{
    if (m_freeIDs == n32nill)
        requestResize();

    uint32_t slotID = m_freeIDs;
    m_freeIDs = ((GameObjectSlot*)m_slot->vector)[slotID].next;

    GameObjectSlot& slot = ((GameObjectSlot*)m_slot->vector)[slotID];
    slot.denseIndex = m_count++;

    GameObjectID cacheID = CreateHandle(slotID, slot.version);

    GameObjectWrapper& wrapper = ((GameObjectWrapper*)m_gameObjectPool->vector)[slot.denseIndex];
    wrapper.slotID = slotID;

    GameObject* gameObject = &wrapper.gameObject;
    new (gameObject) GameObject(cacheID);
    gameObject->setName(name);

    return cacheID;
}

GameObject* GameObjectManager::get(GameObjectID id)
{
    uint32_t slotID = GetHandleID(id);
    uint32_t version = GetHandleVersion(id);

    GameObjectSlot& slot = ((GameObjectSlot*)m_slot->vector)[slotID];
    if (slot.version != version)
        return nullptr;

    GameObjectWrapper* wrapper = (GameObjectWrapper*)m_gameObjectPool->vector;

    return &wrapper[slot.denseIndex].gameObject;
}

void GameObjectManager::destroyImediate(GameObjectID id)
{
    uint32_t slotID = GetHandleID(id);
    uint32_t version = GetHandleVersion(id);

    GameObjectSlot* slots = (GameObjectSlot*)m_slot->vector;
    
    GameObjectSlot& slot = slots[slotID];
    if (slot.version != version)
        return;

    GameObjectWrapper* wrapper = (GameObjectWrapper*)m_gameObjectPool->vector;

    uint32_t removeIndex = slot.denseIndex;

    auto& gameObject = wrapper[removeIndex].gameObject;

    {
        auto transform = gameObject.getTransform();
        for (auto child : transform->getChildren())
            destroyImediate(BehaviorStorage::GetComponent<Transform>(child)->GetGameObject()->getCacheID());
    }

    gameObject.~GameObject();

    uint32_t lastIndex = --m_count;
    if (removeIndex != lastIndex)
    {
        auto& lastWrapper = wrapper[lastIndex];

        new (&gameObject) GameObject(lastWrapper.gameObject);

        wrapper[removeIndex].slotID = wrapper[lastIndex].slotID;

        slots[wrapper[removeIndex].slotID].denseIndex = removeIndex;
    }

    slot.denseIndex = INVALID_COMPONENT_ID;

    slot.next = m_freeIDs;
    m_freeIDs = slotID;

    slot.version++;
}

void GameObjectManager::destroy(GameObjectID id)
{}

void GameObjectManager::requestResize()
{
    std::cout << "Resize go" << std::endl;
    return;
    size_t lastsize = m_gameObjectPool->length;
    size_t newsize = lastsize + 1024;

    // m_gameObjectPool = array_resize(m_gameObjectPool, newsize, sizeof(GameObjectWrapper));

    auto newPool = array_create(newsize, sizeof(GameObjectWrapper));

    GameObjectWrapper* oldWrapperPool = (GameObjectWrapper*)m_gameObjectPool->vector;
    GameObjectWrapper* newWrapperPool = (GameObjectWrapper*)newPool->vector;
    for (size_t i=0;i<lastsize;i++)
    {
        GameObjectWrapper& oldwrapper = oldWrapperPool[i];
        GameObjectWrapper& newwrapper = newWrapperPool[i];

        // newwrapper.next = oldwrapper.next;

        new (&newwrapper.gameObject) GameObject(oldwrapper.gameObject);
    }

    array_free(m_gameObjectPool);
    m_gameObjectPool = newPool;

    addFreeID(lastsize);
}

void GameObjectManager::addFreeID(uint32_t start)
{
    GameObjectSlot* slots = (GameObjectSlot*)m_slot->vector;

    size_t lastID = m_slot->length - 1;
    for (size_t i=start;i<lastID;i++)
        slots[i].next = i + 1;

    slots[lastID].next = m_freeIDs;
    m_freeIDs = start;
}


