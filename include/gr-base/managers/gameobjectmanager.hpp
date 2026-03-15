#pragma once

#include "gr-base/ecs/default.hpp"

#include <vector>
#include <string>
#include <mutex>

extern "C"
{
#include "gr-base/externals/array.h"
}

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

    size_t m_capacity;

    array* m_pool;

    uint32_t m_freeIDs;

    size_t m_count;

    GameObjectHandle instantiate(GameObjectHandle other);

    GameObjectHandle create(const std::string& name);

    GameObject* get(GameObjectHandle id);

    void destroyImediate(GameObjectHandle handle);

    void destroy(GameObjectHandle handle);

    void realloc();

    void addFreeID(uint32_t start);

    void cleanup();
};




