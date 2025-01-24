#include "ecs/manager.hpp"
#include "ecs/entity.hpp"

#include <algorithm>

#define MAX_COMPONENT 16
#define MAX_PAGES 10

std::unordered_map<EntityID, std::unique_ptr<Entity>> Manager::mMapEntities;

std::vector<Entity *> Manager::mEntities;

std::vector<EntityID> Manager::mAvailable;

grm::size Manager::mEntityCount = 0;

std::mutex Manager::mMutex;

Entity *Manager::CreateEntity(GameObject *go) {
    std::lock_guard<std::mutex> lock(mMutex);
    auto hasAvailable = !mAvailable.empty();

    auto id = (hasAvailable ? mAvailable.back() : mEntityCount++);
    if (hasAvailable) {
        mAvailable.pop_back();
    }
    
    auto &entity = mMapEntities[id] = std::make_unique<Entity>(go, id);

    return mEntities.emplace_back(&(*entity));
}

void Manager::DestroyEntity(Entity *entity) {
    auto id = entity->getEntityID();

    std::lock_guard<std::mutex> lock(mMutex);
    mAvailable.push_back(id);

    mMapEntities.erase(id);

    mEntities.erase(std::remove(mEntities.begin(), mEntities.end(), entity), mEntities.end());
}

std::size_t Manager::GetEntityCount() {
    std::lock_guard<std::mutex> lock(mMutex);
    return mEntities.size();
}

void Manager::Update() {
    auto pages = (mEntities.size() + MAX_PAGES - 1) / MAX_PAGES;
    for (auto i=0;i<pages;i++) {
        auto start = i * MAX_PAGES;
        auto end = std::min(start + MAX_PAGES, static_cast<int>(mEntities.size()));

        for (auto j=start;j<end;j++) {
            // if (!mEntities[j]->isActive()) {
            //     continue;
            // }

            mEntities[j]->update();
        }
    }
}
