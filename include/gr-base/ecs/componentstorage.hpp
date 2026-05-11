#pragma once

#include "gr-base/ecs/componentinfo.hpp"
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

    template <typename T>
    static T* GetDenseArray();

    static Component* GetComponent(ComponentHash hash, ComponentHandle id);

    static void NotifyUpdate(ComponentHash hash, ComponentInfo* src);

    static void Start();

    static void Update();

    ComponentStorage();
    ~ComponentStorage();

private:
    static ComponentStorage* s_ComponentStorage;

    ObjectPool<ComponentSlot> m_slots;

    array* m_storage;

    void notifyUpdate(ComponentHash hash, ComponentInfo* src);

    void reallocComponentChunk(ComponentInfo* info, size_t newStride);

    void restoreVTable(ComponentInfo* info, void* vtable);

    ComponentHandle allocate(ComponentHash hash, ComponentHandle source);

    void reallocate(ComponentInfo* info, ComponentSparseSet* set);

    void deallocate(ComponentHash hash, ComponentHandle handle);

    Component* getComponent(ComponentHash hash, ComponentHandle handle);

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
inline T* ComponentStorage::GetDenseArray()
{}

