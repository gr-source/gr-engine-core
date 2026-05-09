#pragma once

#include "componentinfo.hpp"
#include "gr-base/pool/poolObject.hpp"
#include "utils.hpp"

#include <cstddef>
#include <unordered_map>
#include <vector>
#include <bitset>

typedef uint32_t ComponentInfoID;

class ComponentRegistry
{
public:
    template <typename T>
    static bool Registry(const char* name, ComponentHash hash, ComponentHash parent);

    static void Unregistry(ComponentHash hash);

    static ComponentInfo* Get(ComponentHash hash);

    static ComponentInfo* GetAllComponentInfo();

    static ComponentInfoID GetCount();

private:
    static ComponentRegistry& GetInstance();

    std::unordered_map<size_t, ComponentInfoID> m_lookUpMap;

    // std::bitset<COMPONENT_TABLE_SIZE> m_bitmask;

    ComponentInfoID m_capacity;

    ComponentInfoID m_count;

    array* m_pool;

    // array* m_sparseSet;

    ComponentRegistry();
    ~ComponentRegistry();

    bool registry(ComponentInfo* info);

    void unregistry(ComponentHash hash);

    ComponentInfo* get(ComponentHash hash);

    inline ComponentInfo* getAllComponentInfo() const
    {
        return (ComponentInfo*)m_pool->vector;
    }

    inline const ComponentInfoID& getCount() const
    {
        return m_count;
    }

    bool reconfigure(ComponentInfo* dest, const ComponentInfo* src);

    void reallocate();

    template <typename T>
    static void Constructor(Component* dest, const Component* src);

    template <typename T>
    static void Destructor(Component* behavior);

    template <typename T>
    static void* GetVTablePTR();
};

template <typename T>
inline bool ComponentRegistry::Registry(const char* name, ComponentHash hash, ComponentHash parent)
{
    ComponentInfo info;
    info.stride = sizeof(T);
    info.stride = sizeof(struct ComponentWrapper<T>);
    info.name = name;
    info.hash = hash;
    info.parent = parent;
    info.offset = (size_t)((uint8_t*)&((ComponentWrapper<T>*)0)->component);

    info.constructor = (MethodPointer)ComponentRegistry::Constructor<T>;
    info.destructor = (MethodPointer)ComponentRegistry::Destructor<T>;

    info.getvetable = (MethodPointer)ComponentRegistry::GetVTablePTR<T>;

    info.start = has_start<T>::value ? (MethodPointer)StartWrapper<T> : 0;
    info.update = has_update<T>::value ? (MethodPointer)UpdateWrapper<T> : 0;
    // info.destroy = has_destroy<T>::value ? (MethodPointer)GetMethodPointer(&T::Destroy) : 0;

    return GetInstance().registry(&info);
}

// private
template <typename T>
inline void ComponentRegistry::Constructor(Component* dest, const Component* src)
{
    if (src != nullptr)
        new (dest) T(*static_cast<const T*>(src));
    else
        new (dest) T();
}

template <typename T>
inline void ComponentRegistry::Destructor(Component* behavior)
{
    static_cast<T*>(behavior)->~T();
}

template <typename T>
inline void* ComponentRegistry::GetVTablePTR()
{
    alignas(T) char buffer[sizeof(T)];
    T* temp = new (buffer) T();

    void* vptrHandle = *(void**)temp;

    temp->~T();
    return vptrHandle;
}


