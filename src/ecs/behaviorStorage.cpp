#include "gr-base/ecs/behaviorStorage.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/default.hpp"
#include "gr-base/core/gameObject.hpp"
#include "gr-base/ecs/behaviorRegistry.hpp"

#include "gr-base/ecs/behavior.hpp"
#include "gr-base/externals/array.h"
#include <cassert>


BehaviorStorage* BehaviorStorage::s_BehaviorStorage = nullptr;

// public
void BehaviorStorage::SetInstance(BehaviorStorage *behaviorStorage)
{
    s_BehaviorStorage = behaviorStorage;
}

ComponentHandle BehaviorStorage::Allocate(uint64_t hash, ComponentHandle source)
{
    return s_BehaviorStorage->allocate(hash, source);
}

void BehaviorStorage::Deallocate(uint64_t hash, ComponentHandle handle)
{
    return s_BehaviorStorage->deallocate(hash, handle);
}

bool BehaviorStorage::Reinstance(uint64_t hash, Behavior* dest, const Behavior* src)
{
    return s_BehaviorStorage->reinstance(hash, dest, src);
}

Behavior* BehaviorStorage::GetComponent(uint64_t hash, ComponentHandle handle)
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
        set.slot = nullptr;
        set.dense = nullptr;
        set.count = 0;
        set.freeID = INVALID_COMPONENT_ID;
    }
}

BehaviorStorage::~BehaviorStorage()
{
    ComponentSparseSet* vector = (ComponentSparseSet*)m_storage->vector;
    for (uint32_t i=0;i<m_storage->length;i++)
    {
        ComponentSparseSet& set = vector[i];
        if (set.slot != nullptr)
            array_free(set.slot);

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
    size_t storageID = info->hash & (COMPONENT_TABLE_SIZE - 1);

    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];

    uint8_t* ptr = (uint8_t*)(set.dense->vector) + info->offset;

    for (ComponentID id=0;id<set.count;id++)
    {
        Behavior* behavior = (Behavior*)ptr;
        ((void (*)(Behavior*, const Behavior*))(info->constructor))(behavior, behavior);

        ptr += info->stride;
    }
}

ComponentHandle BehaviorStorage::allocate(uint64_t hash, ComponentHandle source)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return HANDLE_INVALID;

    size_t storageID = hash & (COMPONENT_TABLE_SIZE - 1);

    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];
    if (set.freeID == INVALID_COMPONENT_ID)
        resize(info, &set, 512);

    ComponentSlot* slots = (ComponentSlot*)set.slot->vector;

    uint8_t* wraper = (uint8_t*)set.dense->vector;

    ComponentID slotID = set.freeID;
    set.freeID = slots[slotID].next;
    
    ComponentSlot& slot = slots[slotID];
    slot.denseIndex = set.count++;

    *(ComponentID*)(wraper + (slot.denseIndex * info->stride)) = slotID;

    Behavior* behavior = (Behavior*)(wraper + (slot.denseIndex * info->stride + info->offset));
    ((void(*)(Behavior*, const Behavior*))(info->constructor))(behavior, source != HANDLE_INVALID ? GetComponent(hash, source) : nullptr);

    return handle_create(slotID, slot.version);
}

#include <string.h>
void BehaviorStorage::deallocate(uint64_t hash, ComponentHandle handle)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return;

    size_t storageID = hash & (COMPONENT_TABLE_SIZE - 1);
    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];

    ComponentID slotID = handle_get_id(handle);
    ComponentID version = handle_get_version(handle);

    ComponentSlot* slots = (ComponentSlot*)set.slot->vector;
    if (slots == nullptr)
        return;

    ComponentSlot* slot = &slots[slotID];
    if (slot->version != version)
        return;

    uint8_t* wrapper = (uint8_t*)set.dense->vector;

    ComponentID removeIndex = slot->denseIndex;
    ComponentID lastIndex = --set.count;

    Behavior* behavior = (Behavior*)(wrapper + (removeIndex * info->stride + info->offset));
    ((void(*)(Behavior*))(info->destructor))(behavior);

    slot->next = set.freeID;
    set.freeID = slotID;

    if (removeIndex != lastIndex)
    {
        Behavior* lastBehavior = (Behavior*)(wrapper + (lastIndex * info->stride + info->offset));

        ((void(*)(Behavior*, const Behavior*))(info->constructor))(behavior, lastBehavior);

        // memset((void*)lastBehavior, 0, info->size - info->offset);

        ComponentID newSlotID = *(ComponentID*)(wrapper + (lastIndex * info->stride));

        *(ComponentID*)(wrapper + (removeIndex * info->stride)) = newSlotID;

        slots[newSlotID].denseIndex = removeIndex;
    }

    slot->denseIndex = INVALID_COMPONENT_ID;
    slot->version++;
}

Behavior* BehaviorStorage::getComponent(uint64_t hash, ComponentHandle handle)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return nullptr;

    size_t storageID = hash & (COMPONENT_TABLE_SIZE - 1);

    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];

    ComponentID slotID = handle_get_id(handle);
    ComponentID version = handle_get_version(handle);

    ComponentSlot& slot = ((ComponentSlot*)set.slot->vector)[slotID];
    if (slot.version != version)
        return nullptr;

    uint8_t* wrapper = (uint8_t*)(set.dense->vector);
    
    return (Behavior *)(wrapper + (slot.denseIndex * info->stride + info->offset));
}

bool BehaviorStorage::reinstance(uint64_t hash, Behavior* dest, const Behavior* src)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return false;

    // size_t poolID = hash % COMPONENT_TABLE_SIZE;

    // BehaviorPool& pool = ((BehaviorPool*)m_storage->vector)[poolID];

    ((void(*)(Behavior*, const Behavior*))(info->constructor))(dest, src);

    return true;
}

static void AddFreeID(ComponentInfo* info, ComponentSparseSet* set, ComponentID first)
{
    array* arr = set->slot;

    ComponentSlot* slots = (ComponentSlot*)arr->vector;

    ComponentID last = arr->length - 1;
    for (size_t i=first;i<last;i++)
        slots[i].next = i + 1;

    slots[last].next = set->freeID;
    set->freeID = first;
}

void BehaviorStorage::resize(ComponentInfo* info, ComponentSparseSet* set, size_t count)
{
    size_t lastsize = set->dense != nullptr ? set->dense->length : 0;
    // size_t newsize = lastsize + count;

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
        // for (size_t i=0;i<lastsize;i++)
            // *(ComponentID*)(newComponentWrapper + (i * info->size)) = *(ComponentID*)(lastComponentWrapper + (i * info->size));

        for (size_t i=0;i<set->count;i++)
        {
            Behavior* lastbehavior = (Behavior *)((lastComponentWrapper + (i * info->stride)) + info->offset);
            Behavior* newbehavior = (Behavior *)((newComponentWrapper + (i * info->stride)) + info->offset);

            ((void(*)(Behavior*, const Behavior*))(info->constructor))(newbehavior, lastbehavior);
        }

        array_free(set->dense);
        set->dense = newPool;

        auto slot = array_create(set->capacity, sizeof(ComponentSlot));
        memcpy((void*)slot->vector, (void*)set->slot->vector, lastsize * sizeof(ComponentSlot));

        auto slots = (ComponentSlot*)slot->vector;
        for (ComponentID i=lastsize;i<set->capacity;i++)
            slots[i] = {INVALID_COMPONENT_ID, 1, INVALID_COMPONENT_ID};

        array_free(set->slot);
        set->slot = slot;
    } else
    {
        set->capacity = 2;

        set->slot = array_create(set->capacity, sizeof(ComponentSlot));
        set->dense = array_create(set->capacity, info->stride);

        ComponentSlot* slots = (ComponentSlot*)set->slot->vector;
        for (ComponentID i=lastsize;i<set->capacity;i++)
            slots[i] = {INVALID_COMPONENT_ID, 1, INVALID_COMPONENT_ID};
    }

    AddFreeID(info, set, lastsize);
}

void BehaviorStorage::start()
{
    array* infos = BehaviorRegistry::GetBehaviorInfos();

    ComponentSparseSet* vector = (ComponentSparseSet*)m_storage->vector;

    for (auto& hash : BehaviorRegistry::GetActiveBehaviorActive())
    {
        ComponentInfo* info = BehaviorRegistry::Get(hash);
        if (!info->starthash)
            continue;

        size_t storageID = hash & (COMPONENT_TABLE_SIZE - 1);

        ComponentSparseSet& set = vector[storageID];
        if (set.dense == nullptr)
            continue;

        uint8_t* ptr = (uint8_t*)set.dense->vector + info->offset;
        size_t stride = info->stride;

        void** vtable = (void**)ptr;
        if (*vtable == nullptr)
        {
            Console::LogError("[ BehaviorStorage ] the vtable of %s is breaked.", info->name);
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
    array* infos = BehaviorRegistry::GetBehaviorInfos();

    ComponentSparseSet* vector = (ComponentSparseSet*)m_storage->vector;

    for (auto& hash : BehaviorRegistry::GetActiveBehaviorActive())
    {
        ComponentInfo* info = BehaviorRegistry::Get(hash);
        if (!info->update)
            continue;

        size_t storageID = hash & (COMPONENT_TABLE_SIZE - 1);

        ComponentSparseSet& set = vector[storageID];
        if (set.dense == nullptr)
            continue;

        uint8_t* ptr = (uint8_t*)set.dense->vector + info->offset;
        size_t stride = info->stride;

        for (ComponentID i=0;i<set.count;i++)
        {
            Behavior* behavior = (Behavior*)ptr;
            if (behavior->bCanEnableUpdate)
                behavior->Update();

            ptr += stride;
        }
    }
}




