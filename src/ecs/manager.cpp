#include "gr-base/ecs/manager.hpp"
#include "gr-base/ecs/entity.hpp"

#include <algorithm>

#define MAX_COMPONENT 16
#define MAX_PAGES 10

std::unordered_map<EntityID, std::unique_ptr<Entity>> Manager::mMapEntities;

std::vector<Entity *> Manager::mEntities;

std::stack<EntityID> Manager::mAvailable;

grm::size Manager::mEntityCount = 0;

std::mutex Manager::mMutex;

Entity *Manager::CreateEntity(GameObject *go)
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto hasAvailable = !mAvailable.empty();

    auto id = (hasAvailable ? mAvailable.top() : mEntityCount++);
    if (hasAvailable) {
        mAvailable.pop();
    }
    
    auto &entity = mMapEntities[id] = std::make_unique<Entity>(go, id);

    return mEntities.emplace_back(&(*entity));
}

Entity *Manager::GetEntityFromID(EntityID id)
{
    auto it = mMapEntities.find(id);
    if (it == mMapEntities.end()) {
        return nullptr;
    }
    return &(*it->second);
}

void Manager::DestroyEntity(Entity *entity)
{
    auto id = entity->getEntityID();

    std::lock_guard<std::mutex> lock(mMutex);
    mAvailable.emplace(id);

    mMapEntities.erase(id);

    mEntities.erase(std::remove(mEntities.begin(), mEntities.end(), entity), mEntities.end());
}

std::size_t Manager::GetEntityCount() {
    std::lock_guard<std::mutex> lock(mMutex);
    return mEntities.size();
}

void Manager::Update()
{
    return ComponentRegistry::Update();
}



