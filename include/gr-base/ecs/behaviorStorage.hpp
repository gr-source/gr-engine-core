#pragma once

#include "gr-base/ecs/componentinfo.hpp"
#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/utils.hpp"

extern "C"
{
#include "gr-base/externals/array.h"
}

typedef struct ComponentSparseSet
{
    array* slot;
    array* dense;
    ComponentID count;
    ComponentID freeID;
} ComponentSparseSet;

typedef struct ComponentSlot
{
    ComponentID denseIndex;
    ComponentID version;
    ComponentID next;
} ComponentSlot;

class BehaviorStorage
{
public:
    static void SetInstance(BehaviorStorage* behaviorStorage);

    static ComponentHandle Allocate(uint64_t hash, ComponentHandle source);

    static void Deallocate(uint64_t hash, ComponentHandle handle);

    static bool Reinstance(uint64_t hash, Behavior* dest, const Behavior* src);

    template <typename T>
    static T* GetComponent(ComponentHandle id);

    static Behavior* GetComponent(uint64_t hash, ComponentHandle id);

    static void Start();

    static void Update();

    static void Destroy();

    BehaviorStorage();
    ~BehaviorStorage();

private:
    static BehaviorStorage* s_BehaviorStorage;

    array* m_storage;

    ComponentHandle allocate(uint64_t hash, ComponentHandle source);

    void resize(ComponentInfo* info, ComponentSparseSet* set, size_t count);

    void deallocate(uint64_t hash, ComponentHandle handle);

    Behavior* getComponent(uint64_t hash, ComponentHandle handle);

    bool reinstance(uint64_t hash, Behavior* dest, const Behavior* src);

    void start();

    void update();

    void destroy();
};

template <typename T>
inline T* BehaviorStorage::GetComponent(ComponentHandle id)
{
    return static_cast<T*>(s_BehaviorStorage->getComponent(GetHashByType<T>("nill"), id));
}

