#include "gr-base/managers/gameobjectmanager.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/default.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/behaviorStorage.hpp"
#include "gr-base/externals/array.h"
#include "gr-base/script/transform.hpp"

#include "gr-base/managers/sceneManager.hpp"

#include <cassert>
#include <cstddef>
#include <string.h>

typedef struct GameObjectWrapper
{
    uint32_t next;
    uint32_t version;
    GameObject gameObject;
} GameObjectWrapper;

GameObjectManager* GameObjectManager::s_GameObjectManager = nullptr;

void GameObjectManager::SetInstance(GameObjectManager* gameobjectmanager)
{
    s_GameObjectManager = gameobjectmanager;
}

GameObjectHandle GameObjectManager::Instantiate(GameObjectHandle handle)
{
    return s_GameObjectManager->instantiate(handle);
}

GameObjectHandle GameObjectManager::Create(const std::string &name)
{
    return s_GameObjectManager->create(name);
}

GameObject* GameObjectManager::Get(GameObjectHandle id)
{
    return s_GameObjectManager->get(id);
}

void GameObjectManager::DestroyImediate(GameObjectHandle handle)
{
    s_GameObjectManager->destroyImediate(handle);
}

void GameObjectManager::Destroy(GameObjectHandle handle)
{
    s_GameObjectManager->destroy(handle);
}

void GameObjectManager::Cleanup()
{
    s_GameObjectManager->cleanup();
}

size_t GameObjectManager::GetCount()
{
    return s_GameObjectManager->m_count;
}

// public
GameObjectManager::GameObjectManager() : m_pool(nullptr), m_freeIDs(n32nill), m_count(0), m_capacity(0)
{}

GameObjectManager::~GameObjectManager()
{
    if (m_pool != nullptr)
        array_free(m_pool);
}

// private
GameObjectHandle GameObjectManager::instantiate(GameObjectHandle other)
{
    auto newGameObjectID = create(GameObject::Get(other)->GetName());
    if (newGameObjectID == HANDLE_INVALID)
        return HANDLE_INVALID;

    auto otherGO = GameObject::Get(other);
    auto scene = otherGO->GetScene() != nullptr ? otherGO->GetScene() : SceneManager::GetCurrent();

    {
        auto gameObject = get(newGameObjectID);

        gameObject->Copy(other);
    }

    auto gameObject = get(newGameObjectID);

    if (scene != nullptr)
    {
        scene->insert(newGameObjectID);

        gameObject->SetScene(scene);

        if (scene->bRunning)
            gameObject->CallStart();
    }

    return newGameObjectID;
}

GameObjectHandle GameObjectManager::create(const std::string &name)
{
    if (m_freeIDs == n32nill)
        realloc();

    uint32_t id = m_freeIDs;

    GameObjectWrapper* wrapper = (GameObjectWrapper*)m_pool->vector + id;
    m_freeIDs = wrapper->next;

    GameObjectHandle cacheID = handle_create(id, wrapper->version);

    GameObject* gameObject = &wrapper->gameObject;
    new (gameObject) GameObject(cacheID);
    gameObject->SetName(name);

    m_count++;

    return cacheID;
}

GameObject* GameObjectManager::get(GameObjectHandle id)
{
    uint32_t slotID = handle_get_id(id);
    uint32_t version = handle_get_version(id);

    GameObjectWrapper& wrapper = ((GameObjectWrapper*)m_pool->vector)[slotID];
    if (wrapper.version != version)
        return nullptr;

    return &wrapper.gameObject;
}

void GameObjectManager::destroyImediate(GameObjectHandle handle)
{
    uint32_t slotID = handle_get_id(handle);
    uint32_t version = handle_get_version(handle);

    GameObjectWrapper& wrapper = ((GameObjectWrapper*)m_pool->vector)[slotID];
    if (wrapper.version != version)
        return;

    auto& gameObject = wrapper.gameObject;
    gameObject.CallDestroy();

    auto scene = gameObject.GetScene();
    if (scene != nullptr)
        scene->remove(handle);

    {
        auto transform = gameObject.GetTransform();
        for (auto child : transform->getChildren())
            destroyImediate(BehaviorStorage::GetComponent<Transform>(child)->GetGameObject()->GetCacheID());
    }

    gameObject.~GameObject();

    wrapper.next = m_freeIDs;
    m_freeIDs = slotID;

    wrapper.version++;

    m_count--;
}

void GameObjectManager::destroy(GameObjectHandle handle)
{
    m_markForDestruction.push_back(handle);
}

void GameObjectManager::realloc()
{
    size_t lastCapacity = m_capacity;

    if (lastCapacity)
    {
        m_capacity *= 2;

        array* newPool = array_create(m_capacity, sizeof(GameObjectWrapper));

        GameObjectWrapper* lastWrapperPool = (GameObjectWrapper*)m_pool->vector;
        GameObjectWrapper* newWrapperPool = (GameObjectWrapper*)newPool->vector;

        for (size_t i=0;i<lastCapacity;i++)
        {
            GameObjectWrapper& lastWrapper = lastWrapperPool[i];
            GameObjectWrapper& newWrapper = newWrapperPool[i];

            newWrapper.version = lastWrapper.version;
            newWrapper.next = lastWrapper.next;

            new (&newWrapper.gameObject) GameObject(std::move(lastWrapper.gameObject));

            lastWrapper.gameObject.~GameObject();
        }

        for (size_t i=lastCapacity;i<m_capacity;i++)
            newWrapperPool[i].version = 1;

        array_free(m_pool);
        m_pool = newPool;
    } else
    {
        m_capacity = 1;

        m_pool = array_create(m_capacity, sizeof(GameObjectWrapper));

        GameObjectWrapper* wrapper = (GameObjectWrapper*)m_pool->vector;

        for (size_t i=0;i<m_capacity;i++)
            wrapper[i].version = 1;
    }

    addFreeID(lastCapacity);
}

void GameObjectManager::addFreeID(uint32_t start)
{
    GameObjectWrapper* wrapper = (GameObjectWrapper*)m_pool->vector;

    size_t lastID = m_pool->length - 1;
    for (size_t i=start;i<lastID;i++)
        wrapper[i].next = i + 1;

    wrapper[lastID].next = m_freeIDs;
    m_freeIDs = start;
}

void GameObjectManager::cleanup()
{
    for (auto &handle : m_markForDestruction)
        destroyImediate(handle);

    m_markForDestruction.clear();
}

