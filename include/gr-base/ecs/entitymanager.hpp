#pragma once

#include <stack>
#include <vector>
#include <memory>
#include <mutex>

#include "entity.hpp"

class EntityManager
{
public:
    static Entity *Create(GameObject *go);

    static void Destroy(Entity *entity);

    static std::size_t GetCount();

    static void Update();

    static void Initialize();

    static void Shutdown();
    
private:
    static EntityManager *m_instance;

    std::unordered_map<EntityID, std::unique_ptr<Entity>> m_entitymap;

    std::vector<Entity *> m_entitylist;

    std::stack<EntityID> m_freeid;

    EntityID m_entitycount;

    std::mutex m_mutex;

    Entity *create(GameObject *gameObject);

    void destroy(Entity *entity);

    std::size_t get_count();
};

