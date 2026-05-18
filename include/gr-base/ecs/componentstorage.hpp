#pragma once

#include "gr-base/ecs/typeinfo.hpp"
#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/utils.hpp"

#include "gr-base/pool/poolObject.hpp"

extern "C"
{
#include "gr-base/externals/array.h"
}

typedef struct ComponentSlot
{
    ComponentID denseIndex;
    ComponentID version;
} ComponentSlot;

typedef uint32_t SlotID;

class ComponentStorage
{
public:
    static void SetInstance(ComponentStorage* behaviorStorage);

    static ComponentHandle Allocate(ComponentHash hash, ComponentHandle source);

    static void Deallocate(ComponentHash hash, ComponentHandle handle);

    static bool Reinstance(ComponentHash hash, Component* dest, const Component* src);

    template <typename T>
    static T* GetComponent(ComponentHandle handle);

    static Component* GetComponent(ComponentHash hash, ComponentHandle id);

    template <typename T>
    static ComponentSparseSet *GetSparset();

    static void NotifyUpdate(ComponentHash hash, TypeInfo* src);

    static void Start();

    static void Update();

    ComponentStorage();
    ~ComponentStorage();

private:
    static ComponentStorage* s_ComponentStorage;

    ObjectPool<ComponentSlot> m_slots;

    array* m_storage;

    void notifyUpdate(ComponentHash hash, TypeInfo* src);

    void reallocComponentChunk(TypeInfo* info, size_t newStride);

    void restoreVTable(TypeInfo* info, void* vtable);

    ComponentHandle allocate(ComponentHash hash, ComponentHandle source);

    void reallocate(TypeInfo* info, ComponentSparseSet* set);

    void deallocate(ComponentHash hash, ComponentHandle handle);

    Component* getComponent(ComponentHash hash, ComponentHandle handle);

    ComponentSparseSet* getSparset(ComponentHash hash);

    bool reinstance(ComponentHash hash, Component* dest, const Component* src);

    void start();

    void update();
};

template <typename T>
inline T* ComponentStorage::GetComponent(ComponentHandle handle)
{
    return static_cast<T*>(s_ComponentStorage->getComponent(GetHashByType<T>(), handle));
}

template <typename T>
inline ComponentSparseSet *ComponentStorage::GetSparset()
{
    return s_ComponentStorage->getSparset(GetHashByType<T>());
}



