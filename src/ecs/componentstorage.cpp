#include "gr-base/ecs/componentstorage.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/ecs/componentregistry.hpp"
#include "gr-base/ecs/component.hpp"

#include "gr-base/externals/array.h"
#include "gr-base/externals/handle.h"

#include <iostream>

ComponentStorage* ComponentStorage::s_ComponentStorage = nullptr;

// public
void ComponentStorage::SetInstance(ComponentStorage *behaviorStorage)
{
    s_ComponentStorage = behaviorStorage;

    logger::info("[ComponentStorage] Instance assigned 0x%lX", behaviorStorage);
}

ComponentHandle ComponentStorage::Allocate(ComponentHash hash, ComponentHandle source)
{
    return s_ComponentStorage->allocate(hash, source);
}

void ComponentStorage::Deallocate(ComponentHash hash, ComponentHandle handle)
{
    return s_ComponentStorage->deallocate(hash, handle);
}

bool ComponentStorage::Reinstance(ComponentHash hash, Component* dest, const Component* src)
{
    return s_ComponentStorage->reinstance(hash, dest, src);
}

Component* ComponentStorage::GetComponent(ComponentHash hash, ComponentHandle handle)
{
    return s_ComponentStorage->getComponent(hash, handle);
}

void ComponentStorage::NotifyUpdate(ComponentHash hash, TypeInfo* src)
{
    s_ComponentStorage->notifyUpdate(hash, src);
}

void ComponentStorage::Start()
{
    s_ComponentStorage->start();
}

void ComponentStorage::Update()
{
    s_ComponentStorage->update();
}

ComponentStorage::ComponentStorage() : m_storage(nullptr)
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

ComponentStorage::~ComponentStorage()
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
void ComponentStorage::notifyUpdate(ComponentHash hash, TypeInfo* src)
{
    TypeInfo* info = ComponentRegistry::Get(hash);
    if (!info)
        return;

    if (info->constructorhash != src->constructorhash)
        info->constructorhash = src->constructorhash;

    if (info->destructorhash != src->destructorhash)
        info->destructorhash = src->destructorhash;

    if (info->stride != src->stride)
    {
        reallocComponentChunk(info, src->stride);

        if (info->vtable == src->vtable)
            return;
    }

    restoreVTable(info, src->vtable);
}

void ComponentStorage::reallocComponentChunk(TypeInfo* info, size_t newStride)
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

        Component* behavior = (Component*)(newPtr + info->offset);

        ((void (*)(Component*, const Component*))(info->constructor))(behavior, nullptr);

        newPtr += newStride;
        oldPtr += info->stride;
    }

    array_free(set.dense);
    set.dense = newDensePool;

    logger::debug("[ComponentStorage] Component '%s' data reallocated before '%ld' after '%ld'", info->name, info->stride, newStride);

    info->stride = newStride;
}


void ComponentStorage::restoreVTable(TypeInfo* info, void* new_vtable)
{
    size_t storageID = info->hash & (COMPONENT_TABLE_SIZE - 1);

    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];

    uint8_t* ptr = (uint8_t*)(set.dense->vector) + info->offset;

    for (ComponentID id=0;id<set.count;id++)
    {
        void** vtable = (void**)ptr;
        *vtable = new_vtable;

        ptr += info->stride;
    }

    logger::debug("[ComponentStorage] Component '%s' set vtable '0x%lX' to '0x%lX'", info->name, info->vtable, new_vtable);

    info->vtable = new_vtable;
}

ComponentHandle ComponentStorage::allocate(ComponentHash hash, ComponentHandle source)
{
    TypeInfo* info = ComponentRegistry::Get(hash);
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

    Component* behavior = (Component*)(wraper + (slot->denseIndex * info->stride + info->offset));
    ((void(*)(Component*, const Component*))(info->constructor))(behavior, source != HANDLE_INVALID ? GetComponent(hash, source) : nullptr);

    return handle_create(slotID, slot->version);
}

void ComponentStorage::deallocate(ComponentHash hash, ComponentHandle handle)
{
    TypeInfo* info = ComponentRegistry::Get(hash);
    if (info == nullptr)
        return;

    size_t storageID = hash & (COMPONENT_TABLE_SIZE - 1);
    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];

    ComponentID slotID = handle_get_first(handle);
    ComponentID version = handle_get_second(handle);

    ComponentSlot* slot = m_slots.get(slotID);
    if (slot->version != version)
        return;

    uint8_t* wrapper = (uint8_t*)set.dense->vector;

    ComponentID removeIndex = slot->denseIndex;
    ComponentID lastIndex = --set.count;

    Component* behavior = (Component*)(wrapper + (removeIndex * info->stride + info->offset));
    ((void(*)(Component*))(info->destructor))(behavior);

    if (removeIndex != lastIndex)
    {
        Component* lastComponent = (Component*)(wrapper + (lastIndex * info->stride + info->offset));

        ((void(*)(Component*, const Component*))(info->constructor))(behavior, lastComponent);

        // memset((void*)lastComponent, 0, info->size - info->offset);

        ComponentID newSlotID = *(ComponentID*)(wrapper + (lastIndex * info->stride));

        *(ComponentID*)(wrapper + (removeIndex * info->stride)) = newSlotID;

        m_slots.get(newSlotID)->denseIndex = removeIndex;
    }

    m_slots.deallocate(slotID);

    slot->denseIndex = INVALID_COMPONENT_ID;
    slot->version++;
}

Component* ComponentStorage::getComponent(ComponentHash hash, ComponentHandle handle)
{
    TypeInfo* info = ComponentRegistry::Get(hash);
    if (info == nullptr)
        return nullptr;

    ComponentID slotID = handle_get_first(handle);
    ComponentID version = handle_get_second(handle);

    ComponentSlot* slot = m_slots.get(slotID);
    if (slot->version != version)
        return nullptr;

    size_t storageID = hash & (COMPONENT_TABLE_SIZE - 1);

    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];

    uint8_t* wrapper = (uint8_t*)(set.dense->vector);
    
    return (Component *)(wrapper + (slot->denseIndex * info->stride + info->offset));
}

bool ComponentStorage::reinstance(ComponentHash hash, Component* dest, const Component* src)
{
    TypeInfo* info = ComponentRegistry::Get(hash);
    if (info == nullptr)
        return false;

    ((void(*)(Component*, const Component*))(info->constructor))(dest, src);

    return true;
}

void ComponentStorage::reallocate(TypeInfo* info, ComponentSparseSet* set)
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
            Component* lastbehavior = (Component *)((lastComponentWrapper + (i * info->stride)) + info->offset);
            Component* newbehavior = (Component *)((newComponentWrapper + (i * info->stride)) + info->offset);

            ((void(*)(Component*, const Component*))(info->constructor))(newbehavior, lastbehavior);
        }

        array_free(set->dense);
        set->dense = newPool;
    } else
    {
        set->capacity = 2;

        set->dense = array_create(set->capacity, info->stride);
    }
}

void ComponentStorage::start()
{
    TypeInfo* infos = ComponentRegistry::GetAllTypeInfo();

    ComponentSparseSet* vector = (ComponentSparseSet*)m_storage->vector;
    for (TypeInfoID id=0;id<ComponentRegistry::GetCount();id++)
    {
        TypeInfo& info = infos[id];

        size_t storageID = info.hash & (COMPONENT_TABLE_SIZE - 1);

        ComponentSparseSet& set = vector[storageID];
        if (set.dense == nullptr)
            continue;

        uint8_t* ptr = (uint8_t*)(set.dense->vector) + info.offset;
        size_t stride = info.stride;

        void** vtable = (void**)ptr;
        if (*vtable == nullptr)
        {
            Console::LogError("[ ComponentStorage ] the vtable of %s is breaked.", info.name);
            continue;
        }

        for (ComponentID i=0;i<set.count;i++)
        {
            Component* behavior = (Component*)ptr;
            behavior->Start();

            ptr += stride;
        }
    }
}

void ComponentStorage::update()
{
    TypeInfo* infos = ComponentRegistry::GetAllTypeInfo();

    ComponentSparseSet* vector = (ComponentSparseSet*)m_storage->vector;
    for (TypeInfoID id=0;id<ComponentRegistry::GetCount();id++)
    {
        TypeInfo& info = infos[id];

        size_t storageID = info.hash & (COMPONENT_TABLE_SIZE - 1);

        ComponentSparseSet& set = vector[storageID];
        if (set.dense == nullptr)
            continue;

        uint8_t* ptr = (uint8_t*)(set.dense->vector) + info.offset;
        size_t stride = info.stride;

        void** vtable = (void**)ptr;
        if (*vtable == nullptr)
        {
            Console::LogError("[ ComponentStorage ] the vtable of %s is breaked.", info.name);
            continue;
        }

        for (ComponentID i=0;i<set.count;i++)
        {
            Component* behavior = (Component*)ptr;
            behavior->Update();

            ptr += stride;
        }
    }
}




