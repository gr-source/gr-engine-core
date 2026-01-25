#include "gr-base/ecs/behaviorStorage.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/default.hpp"
#include "gr-base/core/gameObject.hpp"
#include "gr-base/ecs/behaviorRegistry.hpp"

#include "gr-base/ecs/behavior.hpp"


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

#include <iostream>

// private
ComponentHandle BehaviorStorage::allocate(uint64_t hash, ComponentHandle source)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return INVALID_COMPONENT_HANDLE;

    size_t storageID = hash % COMPONENT_TABLE_SIZE;

    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];
    if (set.freeID == INVALID_COMPONENT_ID)
        resize(info, &set, 512);

    uint8_t* wraper = (uint8_t*)set.dense->vector;

    ComponentID index = set.freeID;
    set.freeID = *(ComponentID*)(wraper + (index * info->size));
    
    ComponentID denseIndex = set.count++;

    ComponentSlot& slot = ((ComponentSlot*)set.slot->vector)[index];
    slot.denseIndex = denseIndex;

    std::cout << "allocate: " << info->name << " " << index << " " << denseIndex  << " " << slot.version << std::endl;

    Behavior* behavior = (Behavior*)(wraper + (denseIndex * info->size + info->offset));
    ((void(*)(Behavior*, const Behavior*))(info->constructor))(behavior, source != INVALID_COMPONENT_HANDLE ? GetComponent(hash, source) : nullptr);

    return CreateHandle(index, slot.version);
}

void BehaviorStorage::deallocate(uint64_t hash, ComponentHandle handle)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return;

    size_t storageID = hash % COMPONENT_TABLE_SIZE;
    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];

    ComponentID index = GetHandleID(handle);
    ComponentID version = GetHandleVersion(handle);

    ComponentSlot* slots = (ComponentSlot*)set.slot->vector;
    if (slots == nullptr)
        return;

    ComponentSlot* slot = &slots[index];
    if (slot->version != version)
        return;

    uint8_t* wrapper = (uint8_t*)set.dense->vector;

    *(ComponentID*)(wrapper + (index * info->size)) = set.freeID;
    set.freeID = index;

    ComponentID removeIndex = slot->denseIndex;
    ComponentID lastIndex = --set.count;

    Behavior* behavior = (Behavior*)(wrapper + (removeIndex * info->size + info->offset));
    ((void(*)(Behavior*))(info->destructor))(behavior);

    std::cout << "deallocate " << info->name << ":" << removeIndex << std::endl;

    if (removeIndex != lastIndex)
    {
        std::cout << "swap: " << lastIndex << " to: " << removeIndex << std::endl;

        Behavior* lastBehavior = (Behavior*)(wrapper + (lastIndex * info->size + info->offset));

        ((void(*)(Behavior*, const Behavior*))(info->constructor))(behavior, lastBehavior);

        for (uint32_t i = 0; i < set.slot->length; ++i)
        {
            if (slots[i].denseIndex == lastIndex)
            {
                slots[i].denseIndex = removeIndex;
                break;
            }
        }
    }

    slot->denseIndex = INVALID_COMPONENT_ID;
    slot->version++;
}

Behavior* BehaviorStorage::getComponent(uint64_t hash, ComponentHandle handle)
{
    ComponentInfo* info = BehaviorRegistry::Get(hash);
    if (info == nullptr)
        return nullptr;

    size_t storageID = hash % COMPONENT_TABLE_SIZE;

    ComponentSparseSet& set = ((ComponentSparseSet*)m_storage->vector)[storageID];

    ComponentID index = GetHandleID(handle);
    ComponentID version = GetHandleVersion(handle);

    ComponentSlot& slot = ((ComponentSlot*)set.slot->vector)[index];
    if (slot.version != version)
        return nullptr;

    uint8_t* wrapper = (uint8_t*)set.dense->vector;
    
    return (Behavior *)(wrapper + (slot.denseIndex * info->size + info->offset));
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

static void AddFreeID(ComponentInfo* info, ComponentSparseSet* set, ComponentID last)
{
    array* arr = set->dense;

    uint8_t* wraper = (uint8_t*)arr->vector + (last * info->size);
    size_t stride = info->size;

    for (size_t i=last;i<arr->length - 1;i++)
    {
        *(ComponentID*)(wraper) = i + 1;
        wraper += stride;
    }

    *(ComponentID*)(wraper) = set->freeID;
    set->freeID = last;
}

void BehaviorStorage::resize(ComponentInfo* info, ComponentSparseSet* set, size_t count)
{
    size_t lastsize = set->dense != nullptr ? set->dense->length : 0;
    size_t newsize = lastsize + count;

    if (lastsize > 0)
    {
        /*
        auto newPool = array_create(newsize, info->size);

        uint8_t* lastComponentWrapper = (uint8_t*)pool->dense->vector;
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

        array_free(pool->dense);
        pool->dense = newPool;
        */
    } else
    {
        set->slot = array_create(newsize, sizeof(ComponentSlot));
        set->dense = array_create(newsize, info->size);

        ComponentSlot* slot = (ComponentSlot*)set->slot->vector;
        for (ComponentID i=lastsize;i<newsize;i++)
            slot[i] = {INVALID_COMPONENT_ID, 1};
    }

    AddFreeID(info, set, lastsize);
}

void BehaviorStorage::start()
{
    /*
    array* infos = BehaviorRegistry::GetBehaviorInfos();

    BehaviorPool* pools = (BehaviorPool*)m_storage->vector;

    for (auto& hash : BehaviorRegistry::GetActiveBehaviorActive())
    {
        ComponentInfo* info = BehaviorRegistry::Get(hash);
        if (!info->starthash)
            continue;

        size_t poolID = hash % COMPONENT_TABLE_SIZE;

        BehaviorPool& pool = pools[poolID];

        uint8_t* ptr = (uint8_t*)pool.dense->vector + info->offset;
        size_t stride = info->size;


        void** vtable = (void**)ptr;
        if (*vtable == nullptr)
        {
            Console::LogError("[ BehaviorStorage ] the vtable of %s is breaked.", info->name);
            continue;
        }

        std::cout << "Go: " << info->name << " | " << info->start << std::endl;

        for (ComponentID i=0;i<pool.count;i++)
        {
            Behavior* behavior = (Behavior*)ptr;
            behavior->Start();

            ptr += stride;
        }
    }
    */
}

void BehaviorStorage::update()
{
}

void BehaviorStorage::destroy()
{
}


