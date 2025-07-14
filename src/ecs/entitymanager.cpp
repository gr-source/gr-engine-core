#include "gr-base/ecs/entitymanager.hpp"
#include "gr-base/ecs/entity.hpp"

#include <algorithm>

EntityManager *EntityManager::m_instance = nullptr;

Entity *EntityManager::Create(GameObject *go)
{
    return m_instance->create(go);
}

void EntityManager::Destroy(Entity *entity)
{
    return m_instance->destroy(entity);
}

std::size_t EntityManager::GetCount()
{
    return m_instance->get_count();
}

void EntityManager::Update() {
    ComponentRegistry::Update();
}

void EntityManager::Initialize()
{
    if (m_instance == nullptr)
        m_instance = new EntityManager();
}

void EntityManager::Shutdown()
{
    if (m_instance != nullptr)
        delete m_instance;

    m_instance = nullptr;
}

// private

Entity *EntityManager::create(GameObject *gameObject)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto hasAvailable = !m_freeid.empty();

    auto id = (hasAvailable ? m_freeid.top() : m_entitycount++);
    if (hasAvailable) {
        m_freeid.pop();
    }
    
    auto &&[it, added] = m_entitymap.emplace(id, std::make_unique<Entity>(gameObject, id));
    if (added)
    {
        m_entitylist.push_back(it->second.get());
    }

    return m_entitymap[id].get();
}

void EntityManager::destroy(Entity *entity)
{
    auto id = entity->getEntityID();

    std::lock_guard<std::mutex> lock(m_mutex);
    m_freeid.emplace(id);

    m_entitylist.erase(std::remove(m_entitylist.begin(), m_entitylist.end(), entity), m_entitylist.end());

    m_entitymap.erase(id);
}

std::size_t EntityManager::get_count()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_entitylist.size();
}




