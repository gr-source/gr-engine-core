#include "gr-base/ecs/behaviorStorage.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/ecs/behaviorRegistry.hpp"
#include "gr-base/ecs/behavior.hpp"

#include "gr-base/externals/array.h"

#include <iostream>

BehaviorStorage* BehaviorStorage::s_BehaviorStorage = nullptr;

// public
void BehaviorStorage::SetInstance(BehaviorStorage *behaviorStorage)
{
    s_BehaviorStorage = behaviorStorage;
}

ComponentHandle BehaviorStorage::Allocate(ComponentHash hash, ComponentHandle source)
{
    return s_BehaviorStorage->allocate(hash, source);
}

void BehaviorStorage::Deallocate(ComponentHash hash, ComponentHandle handle)
{
    return s_BehaviorStorage->deallocate(hash, handle);
}

bool BehaviorStorage::Reinstance(ComponentHash hash, Behavior* dest, const Behavior* src)
{
    return s_BehaviorStorage->reinstance(hash, dest, src);
}

Behavior* BehaviorStorage::GetComponent(ComponentHash hash, ComponentHandle handle)
{
    return s_BehaviorStorage->getComponent(hash, handle);
}

void BehaviorStorage::NotifyUpdate(ComponentHash hash, ComponentInfo* src)
{
    s_BehaviorStorage->notifyUpdate(hash, src);
}

void BehaviorStorage::Start()
{
    s_BehaviorStorage->start();
}

void BehaviorStorage::Update()
{
    s_BehaviorStorage->update();
}

BehaviorStorage::BehaviorStorage() : m_storage(nullptr)
{
    m_storage = array_create(COMPONENT_TABLE_SIZE, sizeof(ComponentSparseSet));

    ComponentSparseSet* vector = (ComponentSparseSet*)m_storage->vector;
    for (uint32_t i=0;i<m_storage->length;i++)
    {
        ComponentSparseSet& set = vector[i];
        set.dense = nullptr;
        set.capacity = 0;
        set.count = 0;
    }
}

BehaviorStorage::~BehaviorStorage()
{
    ComponentSparseSet* vector = (ComponentSparseSet*)m_storage->vector;
    for (uint32_t i=0;i<m_storage->length;i++)
    {
        ComponentSparseSet& set = vector[i];
        if (set.dense != nullptr)
            array_free(set.dense);
    }

    array_free(m_storage);
}

// private
void BehaviorStorage::notifyUpdate(ComponentHash hash, ComponentInfo* src)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (!info)
        return;

    if (info->constructorhash != src->constructorhash)
        info->constructorhash = src->constructorhash;

    if (info->destructorhash != src->destructorhash)
        info->destructorhash = src->destructorhash;

    if (info->starthash != src->starthash)
        info->starthash = src->starthash;

    if (info->updatehash != src->updatehash)
        info->updatehash = src->updatehash;

    if (info->destroyhash != src->destroyhash)
        info->destroyhash = src->destroyhash;

    if (info->stride != src->stride)
    {
        reallocComponentChunk(info, src->stride);
    } else
    {
        restoreVTable(info);
    }
}

void BehaviorStorage::reallocComponentChunk(ComponentInfo* info, size_t newStride)
{
    size_t storageID = info->hash & (COMPONENT_TABLE_SIZE - 1);

    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];
    if (set.dense == nullptr)
        return;

    array* newDensePool = array_create(set.capacity, newStride);

    uint8_t* newPtr = (uint8_t*)(newDensePool->vector);
    uint8_t* oldPtr = (uint8_t*)(set.dense->vector);

    for (ComponentID id=0;id<set.count;id++)
    {
        *(ComponentID*)(newPtr) = *(ComponentID*)(oldPtr);

        Behavior* behavior = (Behavior*)(newPtr + info->offset);

        ((void (*)(Behavior*, const Behavior*))(info->constructor))(behavior, nullptr);

        newPtr += newStride;
        oldPtr += info->stride;
    }

    array_free(set.dense);
    set.dense = newDensePool;

    info->stride = newStride;
}


void BehaviorStorage::restoreVTable(ComponentInfo* info)
{
    std::cout << "[BehaviorStorage] Restore VTable of: " << info->name << std::endl;

    void* newVTable = ((void*(*)(void))(info->getvetable))();

    size_t storageID = info->hash & (COMPONENT_TABLE_SIZE - 1);

    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];

    uint8_t* ptr = (uint8_t*)(set.dense->vector) + info->offset;

    for (ComponentID id=0;id<set.count;id++)
    {
        void** vtable = (void**)ptr;
        *vtable = newVTable;

        ptr += info->stride;
    }
}

ComponentHandle BehaviorStorage::allocate(ComponentHash hash, ComponentHandle source)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return HANDLE_INVALID;

    size_t storageID = hash & (COMPONENT_TABLE_SIZE - 1);

    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];
    if (set.count >= set.capacity)
        reallocate(info, &set);

    SlotID slotID = m_slots.allocate();

    uint8_t* wraper = (uint8_t*)set.dense->vector;

    ComponentSlot* slot = m_slots.get(slotID);
    if (!slot->version)
        slot->version = 1;

    slot->denseIndex = set.count++;

    *(ComponentID*)(wraper + (slot->denseIndex * info->stride)) = slotID;

    Behavior* behavior = (Behavior*)(wraper + (slot->denseIndex * info->stride + info->offset));
    ((void(*)(Behavior*, const Behavior*))(info->constructor))(behavior, source != HANDLE_INVALID ? GetComponent(hash, source) : nullptr);

    return handle_create(slotID, slot->version);
}

void BehaviorStorage::deallocate(ComponentHash hash, ComponentHandle handle)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return;

    size_t storageID = hash & (COMPONENT_TABLE_SIZE - 1);
    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];

    ComponentID slotID = handle_get_id(handle);
    ComponentID version = handle_get_version(handle);

    ComponentSlot* slot = m_slots.get(slotID);
    if (slot->version != version)
        return;

    uint8_t* wrapper = (uint8_t*)set.dense->vector;

    ComponentID removeIndex = slot->denseIndex;
    ComponentID lastIndex = --set.count;

    Behavior* behavior = (Behavior*)(wrapper + (removeIndex * info->stride + info->offset));
    ((void(*)(Behavior*))(info->destructor))(behavior);

    if (removeIndex != lastIndex)
    {
        Behavior* lastBehavior = (Behavior*)(wrapper + (lastIndex * info->stride + info->offset));

        ((void(*)(Behavior*, const Behavior*))(info->constructor))(behavior, lastBehavior);

        // memset((void*)lastBehavior, 0, info->size - info->offset);

        ComponentID newSlotID = *(ComponentID*)(wrapper + (lastIndex * info->stride));

        *(ComponentID*)(wrapper + (removeIndex * info->stride)) = newSlotID;

        m_slots.get(newSlotID)->denseIndex = removeIndex;
    }

    m_slots.deallocate(slotID);

    slot->denseIndex = INVALID_COMPONENT_ID;
    slot->version++;
}

Behavior* BehaviorStorage::getComponent(ComponentHash hash, ComponentHandle handle)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return nullptr;

    size_t storageID = hash & (COMPONENT_TABLE_SIZE - 1);

    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];

    ComponentID slotID = handle_get_id(handle);
    ComponentID version = handle_get_version(handle);

    ComponentSlot* slot = m_slots.get(slotID);
    if (slot->version != version)
        return nullptr;

    uint8_t* wrapper = (uint8_t*)(set.dense->vector);
    
    return (Behavior *)(wrapper + (slot->denseIndex * info->stride + info->offset));
}

bool BehaviorStorage::reinstance(ComponentHash hash, Behavior* dest, const Behavior* src)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return false;

    ((void(*)(Behavior*, const Behavior*))(info->constructor))(dest, src);

    return true;
}

void BehaviorStorage::reallocate(ComponentInfo* info, ComponentSparseSet* set)
{
    size_t lastsize = set->dense != nullptr ? set->dense->length : 0;

    if (lastsize > 0)
    {
        set->capacity *= 2;

        auto newPool = array_create(set->capacity, info->stride);

        uint8_t* lastComponentWrapper = (uint8_t*)set->dense->vector;
        uint8_t* newComponentWrapper = (uint8_t*)newPool->vector;

        uint8_t *lastFreeList = lastComponentWrapper;
        uint8_t *newFreeList = newComponentWrapper;
        for (size_t i=0;i<lastsize;i++)
        {
            *(ComponentID*)(newFreeList) = *(ComponentID*)(lastFreeList);

            newFreeList += info->stride;
            lastFreeList += info->stride;
        }

        for (size_t i=0;i<set->count;i++)
        {
            Behavior* lastbehavior = (Behavior *)((lastComponentWrapper + (i * info->stride)) + info->offset);
            Behavior* newbehavior = (Behavior *)((newComponentWrapper + (i * info->stride)) + info->offset);

            ((void(*)(Behavior*, const Behavior*))(info->constructor))(newbehavior, lastbehavior);
        }

        array_free(set->dense);
        set->dense = newPool;
    } else
    {
        set->capacity = 2;

        set->dense = array_create(set->capacity, info->stride);
    }
}

void BehaviorStorage::start()
{
    ComponentInfo* infos = BehaviorRegistry::GetAllComponentInfo();

    ComponentSparseSet* vector = (ComponentSparseSet*)m_storage->vector;
    for (ComponentInfoID id=0;id<BehaviorRegistry::GetCount();id++)
    {
        ComponentInfo& info = infos[id];
        if (!info.starthash)
            continue;

        size_t storageID = info.hash & (COMPONENT_TABLE_SIZE - 1);

        ComponentSparseSet& set = vector[storageID];
        if (set.dense == nullptr)
            continue;

        uint8_t* ptr = (uint8_t*)(set.dense->vector) + info.offset;
        size_t stride = info.stride;

        void** vtable = (void**)ptr;
        if (*vtable == nullptr)
        {
            Console::LogError("[ BehaviorStorage ] the vtable of %s is breaked.", info.name);
            continue;
        }

        for (ComponentID i=0;i<set.count;i++)
        {
            Behavior* behavior = (Behavior*)ptr;
            behavior->Start();

            ptr += stride;
        }
    }
}

void BehaviorStorage::update()
{
    ComponentInfo* infos = BehaviorRegistry::GetAllComponentInfo();

    ComponentSparseSet* vector = (ComponentSparseSet*)m_storage->vector;
    for (ComponentInfoID id=0;id<BehaviorRegistry::GetCount();id++)
    {
        ComponentInfo& info = infos[id];
        if (!info.update)
            continue;

        size_t storageID = info.hash & (COMPONENT_TABLE_SIZE - 1);

        ComponentSparseSet& set = vector[storageID];
        if (set.dense == nullptr)
            continue;

        uint8_t* ptr = (uint8_t*)(set.dense->vector) + info.offset;
        size_t stride = info.stride;

        void** vtable = (void**)ptr;
        if (*vtable == nullptr)
        {
            Console::LogError("[ BehaviorStorage ] the vtable of %s is breaked.", info.name);
            continue;
        }

        for (ComponentID i=0;i<set.count;i++)
        {
            Behavior* behavior = (Behavior*)ptr;
            behavior->Update();

            ptr += stride;
        }
    }
}




