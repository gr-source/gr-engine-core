#include "gr-base/ecs/componentmanager.hpp"

#include "gr-base/ecs/behavior.hpp"

#include <algorithm>

#include <string.h>

// public
void ComponentManager::Unregistry(uint64_t hash)
{
    GetInstance().unregistry(hash);
}

ComponentID ComponentManager::Allocate(uint64_t hash, ComponentID source)
{
    return GetInstance().allocate(hash, source);
}

void ComponentManager::Deallocate(uint64_t hash, ComponentID id)
{
    return GetInstance().deallocate(hash, id);
}

bool ComponentManager::Reinstance(uint64_t hash, Behavior* dest, const Behavior* src)
{
    return GetInstance().reinstance(hash, dest, src);
}

Behavior* ComponentManager::GetComponent(uint64_t hash, ComponentID id)
{
    return GetInstance().getComponent(hash, id);
}

void ComponentManager::Update()
{
    GetInstance().update();
}

std::vector<size_t> ComponentManager::GetActiveBehaviorIDs()
{
    return GetInstance().m_componentPoolIDs;
}

array* ComponentManager::GetBehaviorInfos()
{
    return GetInstance().m_infos;
}

// private
ComponentManager& ComponentManager::GetInstance()
{
    static ComponentManager s_ComponentManager;
    return s_ComponentManager;
}

ComponentManager::ComponentManager() : m_infos(nullptr)
{
    m_infos = array_create(COMPONENT_TABLE_SIZE, sizeof(ComponentInfo));
}

ComponentManager::~ComponentManager()
{
    ComponentInfo* infos = (ComponentInfo*)m_infos->vector;
    for (auto& poolID : m_componentPoolIDs)
    {
        ComponentInfo& info = infos[poolID];
        if (info.pool != nullptr)
            array_free(info.pool);
    }

    array_free(m_infos);
}

static void AddFreeID(ComponentInfo* info, ComponentID last)
{
    array* pool = info->pool;

    uint8_t* ptr = (uint8_t*)pool->vector + (last * info->size);
    size_t stride = info->size;

    for (size_t i=last;i<pool->length - 1;i++)
    {
        *(ComponentID*)(ptr) = i + 1;
        ptr += stride;
    }

    *(ComponentID*)(ptr) = info->freeID;
    info->freeID = last;
}

void ComponentManager::registry(ComponentInfo* info)
{
    size_t poolID = info->hash % COMPONENT_TABLE_SIZE;
    if (m_bitmask.test(poolID))
    {
        ComponentInfo& next = ((ComponentInfo*)m_infos->vector)[poolID];

        if (replace(&next, info))
            return;
        return;
    }
    m_bitmask.set(poolID, true);

    m_componentPoolIDs.push_back(poolID);

    ComponentInfo& next = ((ComponentInfo*)m_infos->vector)[poolID];
    memcpy((void *)&next, (void *)info, sizeof(ComponentInfo));

    next.pool = array_create(COMPONENT_CHUNK_SIZE, next.size);
    next.freeID = INVALID_COMPONENT_ID;
    next.count = 0;

    AddFreeID(&next, 0);
}

void ComponentManager::unregistry(uint64_t hash)
{
    size_t poolID = hash % COMPONENT_TABLE_SIZE;
    if (!m_bitmask.test(poolID))
        return;
    m_bitmask.set(poolID, false);

    ComponentInfo& info = ((ComponentInfo*)m_infos->vector)[poolID];
    if (info.pool != NULL)
        array_free(info.pool);

    m_componentPoolIDs.erase(std::remove(m_componentPoolIDs.begin(), m_componentPoolIDs.end(), poolID), m_componentPoolIDs.end());
}

static void ResizeComponentPool(ComponentInfo* info, size_t newsize)
{
    size_t lastsize = info->size;

    size_t length = info->pool->length;

    array* newpool = array_create(length, newsize);
    if (newpool == NULL)
        return;

    uint8_t* afterpool = (uint8_t*)info->pool->vector;
    uint8_t* beforepool = (uint8_t*)newpool->vector;

    for (size_t i=0;i<length;i++)
    {
        ComponentID* sibling = (ComponentID*)(afterpool + (i * (lastsize)));

        *(ComponentID*)(beforepool + (i * newsize)) = *sibling;
    }

    array_free(info->pool);
    info->pool = newpool;

    info->size = newsize;
}

bool ComponentManager::replace(ComponentInfo* dest, const ComponentInfo* src)
{
    dest->name = src->name;

    dest->start = src->start;
    dest->update = src->update;
    dest->destroy = src->destroy;
    dest->constructorhash = src->constructorhash;
    dest->destructorhash = src->destructorhash;

    if (dest->size != src->size)
    {
        ResizeComponentPool(dest, src->size);
        return true;
    }

    return false;
}

#include <iostream>

// TODO: Precisa otimizar
static void ResizeComponentArray(ComponentInfo* info, size_t count)
{
    size_t lastsize = info->pool->length;
    size_t newsize = lastsize + count;

    std::cout << "Resize " << info->name << " | " << lastsize << " | " << newsize << std::endl;

    auto newPool = array_create(newsize, info->size);

    uint8_t* lastComponentWrapper = (uint8_t*)info->pool->vector;
    uint8_t* newComponentWrapper = (uint8_t*)newPool->vector;

    for (size_t i=0;i<lastsize;i++)
    {
        {
            *(ComponentID*)(newComponentWrapper + (i * info->size)) = *(ComponentID*)(lastComponentWrapper + (i * info->size));
        }
        Behavior* lastbehavior = (Behavior *)((lastComponentWrapper + (i * info->size)) + info->offset);
        Behavior* newbehavior = (Behavior *)((newComponentWrapper + (i * info->size)) + info->offset);

        ((void(*)(Behavior*, const Behavior*))(info->constructor))(newbehavior, lastbehavior);
    }

    array_free(info->pool);
    info->pool = newPool;

    AddFreeID(info, lastsize);
}

ComponentID ComponentManager::allocate(uint64_t hash, ComponentID source)
{
    size_t poolID = hash % COMPONENT_TABLE_SIZE;
    if (!m_bitmask.test(poolID))
        return INVALID_COMPONENT_ID;

    ComponentInfo& info = ((ComponentInfo*)m_infos->vector)[poolID];

    ComponentID id;
    while ((id = info.freeID) == INVALID_COMPONENT_ID)
        ResizeComponentArray(&info, COMPONENT_CHUNK_SIZE);

    info.count++;

    uint8_t* componentWrapper = (uint8_t*)info.pool->vector;

    ComponentID* sibling = (ComponentID*)(componentWrapper + (id * info.size));
    info.freeID = *sibling;
    *sibling = INVALID_COMPONENT_ID;

    Behavior* behavior = (Behavior *)((componentWrapper + (id * info.size)) + info.offset);
    ((void(*)(Behavior*, const Behavior*))(info.constructor))(behavior, source != INVALID_COMPONENT_ID ? GetComponent(hash, source) : nullptr);

    return id;
}

void ComponentManager::deallocate(uint64_t hash, ComponentID id)
{
    size_t poolID = hash % COMPONENT_TABLE_SIZE;
    if (!m_bitmask.test(poolID))
        return;

    ComponentInfo& info = ((ComponentInfo*)m_infos->vector)[poolID];
    info.count--;
    
    uint8_t* componentWrapper = (uint8_t*)info.pool->vector;

    ComponentID* sibling = (ComponentID*)(componentWrapper + (id * info.size));
    *sibling = info.freeID;
    info.freeID = id;

    ((void(*)(uint8_t*))(info.destructor))((uint8_t*)sibling + info.offset);
}

Behavior* ComponentManager::getComponent(uint64_t hash, ComponentID id)
{
    size_t poolID = hash % COMPONENT_TABLE_SIZE;
    if (!m_bitmask.test(poolID))
        return nullptr;

    ComponentInfo& info = ((ComponentInfo*)m_infos->vector)[poolID];

    uint8_t* componentWrapper = (uint8_t*)info.pool->vector;
    
    return (Behavior *)((componentWrapper + (id * info.size)) + info.offset);
}

bool ComponentManager::reinstance(uint64_t hash, Behavior* dest, const Behavior* src)
{
    size_t poolID = hash % COMPONENT_TABLE_SIZE;
    if (!m_bitmask.test(poolID))
        return false;

    ComponentInfo& info = ((ComponentInfo*)m_infos->vector)[poolID];

    ((void(*)(Behavior*, const Behavior*))(info.constructor))(dest, src);

    return true;
}

void ComponentManager::update()
{
    for (auto& poolID : m_componentPoolIDs)
    {
        ComponentInfo& info = ((ComponentInfo*)m_infos->vector)[poolID];
        if (!info.updatehash)
            continue;

        uint8_t* ptr = (uint8_t*)info.pool->vector + info.offset;
        size_t stride = info.size;

        for (ComponentID i=0;i<info.count;i++)
        {
            Behavior* behavior = (Behavior*)ptr;
            behavior->Update();

            ptr += stride;
        }
    }
}



