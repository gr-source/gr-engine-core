#pragma once

#include <vector>
#include <memory>
#include <mutex>

#include "entity.hpp"

class Manager {
public:
    Manager() = delete;
    ~Manager() = delete;

    static Entity *CreateEntity(GameObject *go);

    static Entity *GetEntityFromID(EntityID id);

    static void DestroyEntity(Entity *entity);

    static std::size_t GetEntityCount();

    static void Update();
    
private:
    static std::unordered_map<EntityID, std::unique_ptr<Entity>> mMapEntities;

    static std::vector<Entity *> mEntities;

    static std::vector<EntityID> mAvailable;

    static grm::size mEntityCount;

    static std::mutex mMutex;
};

