#pragma once

#include "gr-base/core/default.hpp"
#include <vector>

extern "C"
{
#include "gr-base/externals/array.h"
}

#include <string>
#include <atomic>
#include <mutex>

#define n32nill (uint32_t)-1

class GameObjectManager
{
public:
    static void SetInstance(GameObjectManager* gameObjectManager);

    static GameObjectHandle Instantiate(GameObjectHandle handle);

    static GameObjectHandle Create(const std::string& name);

    static GameObject* Get(GameObjectHandle id);

    static void DestroyImediate(GameObjectHandle handle);

    static void Destroy(GameObjectHandle handle);

    static void Cleanup();

    static size_t GetCount();

    GameObjectManager();
    ~GameObjectManager();

protected:
    static GameObjectManager* s_GameObjectManager;

    std::vector<GameObjectHandle> m_markForDestruction;

    std::mutex m_mutex;

    array* m_gameObjectPool;
    array* m_slot;

    uint32_t m_freeIDs;

    std::atomic<size_t> m_count;

    GameObjectHandle instantiate(GameObjectHandle other);

    GameObjectHandle create(const std::string& name);

    GameObject* get(GameObjectHandle id);

    void destroyImediate(GameObjectHandle handle);

    void destroy(GameObjectHandle handle);

    void requestResize();

    void addFreeID(uint32_t start);

    void cleanup();
};




