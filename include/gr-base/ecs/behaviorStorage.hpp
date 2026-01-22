#pragma once

#include "gr-base/ecs/componentinfo.hpp"
#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/utils.hpp"

extern "C"
{
#include "gr-base/externals/array.h"
}

typedef struct BehaviorPool
{
    array* elements;
    array* sparse;
    size_t count;
    ComponentID freeID;
} BehaviorPool;

class BehaviorStorage
{
public:
    static void SetInstance(BehaviorStorage* behaviorStorage);

    static ComponentID Allocate(uint64_t hash, ComponentID source);

    static void Deallocate(uint64_t hash, ComponentID id);

    static bool Reinstance(uint64_t hash, Behavior* dest, const Behavior* src);

    template <typename T>
    static T* GetComponent(ComponentID id);

    static Behavior* GetComponent(uint64_t hash, ComponentID id);

    static void Start();

    static void Update();

    static void Destroy();

    BehaviorStorage();
    ~BehaviorStorage();

private:
    static BehaviorStorage* s_BehaviorStorage;

    array* m_storage;

    ComponentID allocate(uint64_t hash, ComponentID source);

    void resize(ComponentInfo* info, BehaviorPool* pool, size_t count);

    void deallocate(uint64_t hash, ComponentID id);

    Behavior* getComponent(uint64_t hash, ComponentID id);

    bool reinstance(uint64_t hash, Behavior* dest, const Behavior* src);

    void start();

    void update();

    void destroy();
};

template <typename T>
inline T* BehaviorStorage::GetComponent(ComponentID id)
{
    return static_cast<T*>(s_BehaviorStorage->getComponent(GetHashByType<T>("nill"), id));
}

