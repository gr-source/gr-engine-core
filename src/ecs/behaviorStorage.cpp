#include "gr-base/ecs/behaviorStorage.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/ecs/behaviorRegistry.hpp"

#include "gr-base/ecs/behavior.hpp"
#include "gr-base/externals/array.h"


BehaviorStorage* BehaviorStorage::s_BehaviorStorage = nullptr;

// public
void BehaviorStorage::SetInstance(BehaviorStorage *behaviorStorage)
{
    s_BehaviorStorage = behaviorStorage;
}

ComponentID BehaviorStorage::Allocate(uint64_t hash, ComponentID source)
{
    return s_BehaviorStorage->allocate(hash, source);
}

void BehaviorStorage::Deallocate(uint64_t hash, ComponentID id)
{
    return s_BehaviorStorage->deallocate(hash, id);
}

bool BehaviorStorage::Reinstance(uint64_t hash, Behavior* dest, const Behavior* src)
{
    return s_BehaviorStorage->reinstance(hash, dest, src);
}

Behavior* BehaviorStorage::GetComponent(uint64_t hash, ComponentID id)
{
    return s_BehaviorStorage->getComponent(hash, id);
}

void BehaviorStorage::Start()
{
    s_BehaviorStorage->start();
}

void BehaviorStorage::Update()
{
    s_BehaviorStorage->update();
}

void BehaviorStorage::Destroy()
{
    s_BehaviorStorage->destroy();
}

BehaviorStorage::BehaviorStorage() : m_storage(nullptr)
{
    m_storage = array_create(COMPONENT_TABLE_SIZE, sizeof(BehaviorPool));

    BehaviorPool* vector = (BehaviorPool*)m_storage->vector;
    for (uint32_t i=0;i<m_storage->length;i++)
    {
        BehaviorPool& pool = vector[i];
        pool.elements = nullptr;
        pool.sparse = nullptr;
        pool.count = 0;
        pool.freeID = INVALID_COMPONENT_ID;
    }
}

BehaviorStorage::~BehaviorStorage()
{

    BehaviorPool* vector = (BehaviorPool*)m_storage->vector;
    for (uint32_t i=0;i<m_storage->length;i++)
    {
        BehaviorPool& pool = vector[i];
        if (pool.elements != nullptr)
            array_free(pool.elements);

        if (pool.sparse != nullptr)
            array_free(pool.sparse);
    }

    if (m_storage != nullptr)
        array_free(m_storage);
}

#include <iostream>

// private
ComponentID BehaviorStorage::allocate(uint64_t hash, ComponentID source)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return INVALID_COMPONENT_ID;

    size_t poolID = hash % COMPONENT_TABLE_SIZE;

    BehaviorPool& pool = ((BehaviorPool*)m_storage->vector)[poolID];
    if (pool.freeID == INVALID_COMPONENT_ID)
        resize(info, &pool, COMPONENT_CHUNK_SIZE);

    ComponentID id = pool.freeID;
    pool.freeID = *((ComponentID*(*)(void*, ComponentID))(info->getwrappernext))(pool.elements->vector, id);

    ComponentID* sparse = (ComponentID*)pool.sparse->vector;
    sparse[id] = id;

    // std::cout << this << "(allocate): " << info->name << " | " << id << std::endl;

    /*
    uint8_t* componentWrapper = (uint8_t*)pool.elements->vector;

    ComponentID* sibling = (ComponentID*)(componentWrapper + (id * info->size));
    pool.freeID = *sibling;
    *sibling = INVALID_COMPONENT_ID;
    */

    pool.count++;

    Behavior* behavior = ((Behavior*(*)(void*, ComponentID))(info->getwrapperbehavior))(pool.elements->vector, id);// (Behavior *)((componentWrapper + (id * info->size)) + info->offset);
    ((void(*)(Behavior*, const Behavior*))(info->constructor))(behavior, source != INVALID_COMPONENT_ID ? GetComponent(hash, source) : nullptr);

    void** vtable = (void**)behavior;
    // std::cout << info->name << ": " << (*vtable != nullptr ? "Ok" : "No") << std::endl;

    return id;
}

#include <string.h>

void BehaviorStorage::deallocate(uint64_t hash, ComponentID id)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return;

    size_t poolID = hash % COMPONENT_TABLE_SIZE;

    BehaviorPool& pool = ((BehaviorPool*)m_storage->vector)[poolID];
    ComponentID* sparse = (ComponentID*)pool.sparse->vector;

    uint8_t* componentWrapper = (uint8_t*)pool.elements->vector;

    ComponentID indexToRemove = sparse[id];
    ComponentID lastIndex = pool.count - 1;

    ComponentID* sibling = (ComponentID*)(componentWrapper + (indexToRemove * info->size));
    *sibling = pool.freeID;
    pool.freeID = indexToRemove;

    ((void(*)(uint8_t*))(info->destructor))((uint8_t*)sibling + info->offset);

    if (lastIndex != indexToRemove)
    {
        Behavior* to = (Behavior*)(componentWrapper + ((indexToRemove * info->size) + info->offset));
        Behavior* from = (Behavior*)(componentWrapper + ((lastIndex * info->size) + info->offset));

        ((void(*)(Behavior*, const Behavior*))(info->constructor))(to, from);

        sparse[lastIndex] = indexToRemove;
    }
    pool.count--;

}

Behavior* BehaviorStorage::getComponent(uint64_t hash, ComponentID id)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return nullptr;

    size_t poolID = hash % COMPONENT_TABLE_SIZE;

    BehaviorPool& pool = ((BehaviorPool*)m_storage->vector)[poolID];

    uint8_t* componentWrapper = (uint8_t*)pool.elements->vector;

    ComponentID* sparse = (ComponentID*)pool.sparse->vector;
    
    return (Behavior *)((componentWrapper + (sparse[id] * info->size)) + info->offset);
}

bool BehaviorStorage::reinstance(uint64_t hash, Behavior* dest, const Behavior* src)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return false;

    size_t poolID = hash % COMPONENT_TABLE_SIZE;

    BehaviorPool& pool = ((BehaviorPool*)m_storage->vector)[poolID];

    ((void(*)(Behavior*, const Behavior*))(info->constructor))(dest, src);

    return true;
}

static void AddFreeID(ComponentInfo* info, BehaviorPool* pool, ComponentID last)
{
    array* arr = pool->elements;

    uint8_t* ptr = (uint8_t*)arr->vector + (last * info->size);
    size_t stride = info->size;

    for (size_t i=last;i<arr->length - 1;i++)
    {
        *(ComponentID*)(ptr) = i + 1;
        ptr += stride;
    }

    *(ComponentID*)(ptr) = pool->freeID;
    pool->freeID = last;
}

void BehaviorStorage::resize(ComponentInfo* info, BehaviorPool* pool, size_t count)
{
    size_t lastsize = pool->elements != nullptr ? pool->elements->length : 0;
    size_t newsize = lastsize + count;

    if (lastsize > 0)
    {
        auto newPool = array_create(newsize, info->size);

        uint8_t* lastComponentWrapper = (uint8_t*)pool->elements->vector;
        uint8_t* newComponentWrapper = (uint8_t*)newPool->vector;

        for (size_t i=0;i<lastsize;i++)
        {
            {
                *(ComponentID*)(newComponentWrapper + (i * info->size)) = *(ComponentID*)(lastComponentWrapper + (i * info->size));
            }
            Behavior* lastbehavior = (Behavior *)(lastComponentWrapper + ((i * info->size) + info->offset));
            Behavior* newbehavior = (Behavior *)(newComponentWrapper + ((i * info->size) + info->offset));

            ((void(*)(Behavior*, const Behavior*))(info->constructor))(newbehavior, lastbehavior);
        }

        array_free(pool->elements);
        pool->elements = newPool;

        pool->sparse = array_resize(pool->sparse, newsize, sizeof(ComponentID));
    } else
    {
        pool->elements = array_create(newsize, info->size);
        pool->sparse = array_create(newsize, sizeof(ComponentID));
    }

    AddFreeID(info, pool, lastsize);
}

void BehaviorStorage::start()
{
    array* infos = BehaviorRegistry::GetBehaviorInfos();

    BehaviorPool* pools = (BehaviorPool*)m_storage->vector;

    for (auto& hash : BehaviorRegistry::GetActiveBehaviorActive())
    {
        ComponentInfo* info = BehaviorRegistry::Get(hash);
        if (!info->starthash)
            continue;

        size_t poolID = hash % COMPONENT_TABLE_SIZE;

        BehaviorPool& pool = pools[poolID];
        if (!pool.count)
            continue;

        uint8_t* ptr = (uint8_t*)pool.elements->vector + info->offset;
        size_t stride = info->size;

        for (ComponentID i=0;i<pool.count;i++)
        {
            Behavior* behavior = (Behavior*)ptr;
            behavior->Start();

            ptr += stride;
        }
    }
}

void BehaviorStorage::update()
{
    array* infos = BehaviorRegistry::GetBehaviorInfos();

    BehaviorPool* pools = (BehaviorPool*)m_storage->vector;

    for (auto& hash : BehaviorRegistry::GetActiveBehaviorActive())
    {
        ComponentInfo* info = BehaviorRegistry::Get(hash);
        if (!info->updatehash)
            continue;

        size_t poolID = hash % COMPONENT_TABLE_SIZE;

        BehaviorPool& pool = pools[poolID];
        if (!pool.count)
            continue;

        uint8_t* ptr = (uint8_t*)pool.elements->vector + info->offset;
        size_t stride = info->size;

        for (ComponentID i=0;i<pool.count;i++)
        {
            Behavior* behavior = (Behavior*)ptr;
            behavior->Update();

            ptr += stride;
        }
    }
}

void BehaviorStorage::destroy()
{
}


