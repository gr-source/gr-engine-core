#pragma once

#include "gr-base/ecs/typeinfo.hpp"
#include "gr-base/ecs/utils.hpp"

#include "gr-base/pool/poolObject.hpp"

#include <unordered_map>

typedef uint32_t TypeInfoID;

class ComponentRegistry
{
public:
    template <typename T>
    static bool Registry(const char* name, ComponentHash parent);

    static void Unregistry(ComponentHash hash);

    static TypeInfo* Get(ComponentHash hash);

    static TypeInfo* GetAllTypeInfo();

    static TypeInfoID GetCount();

private:
    static ComponentRegistry& GetInstance();

    std::unordered_map<size_t, TypeInfoID> m_lookUpMap;

    // std::bitset<COMPONENT_TABLE_SIZE> m_bitmask;

    TypeInfoID m_capacity;

    TypeInfoID m_count;

    array* m_pool;

    // array* m_sparseSet;

    ComponentRegistry();
    ~ComponentRegistry();

    bool registry(TypeInfo* info);

    void unregistry(ComponentHash hash);

    TypeInfo* get(ComponentHash hash);

    inline TypeInfo* getAllTypeInfo() const
    {
        return (TypeInfo*)m_pool->vector;
    }

    inline const TypeInfoID& getCount() const
    {
        return m_count;
    }

    bool reconfigure(TypeInfo* dest, const TypeInfo* src);

    void reallocate();

    template <typename T>
    static void Constructor(Component* dest, const Component* src);

    template <typename T>
    static void Destructor(Component* behavior);

    template <typename T>
    static void* GetVTablePTR();
};

template <typename T>
inline bool ComponentRegistry::Registry(const char* name, ComponentHash parent)
{
    TypeInfo info;
    info.vtable = ComponentRegistry::GetVTablePTR<T>();
    info.stride = sizeof(struct ComponentWrapper<T>);
    info.name = name;
    info.hash = GetHashByType<T>(name);
    info.parent = parent;
    info.offset = (size_t)((uint8_t*)&((ComponentWrapper<T>*)0)->component);

    info.constructor = (MethodPointer)ComponentRegistry::Constructor<T>;
    info.destructor = (MethodPointer)ComponentRegistry::Destructor<T>;

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


