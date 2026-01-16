#pragma once

#include "componentinfo.hpp"
#include "utils.hpp"

#include <cstddef>
#include <vector>
#include <bitset>

class ComponentManager
{
public:
    template <typename T>
    static ComponentInfo Registry(const char* name, uint64_t hash);

    static void Unregistry(uint64_t hash);

    static ComponentID Allocate(uint64_t hash, ComponentID source);

    static void Deallocate(uint64_t hash, ComponentID id);

    static bool Reinstance(uint64_t hash, Behavior* dest, const Behavior* src);

    template <typename T>
    static T* GetComponent(ComponentID id);

    static Behavior* GetComponent(uint64_t hash, ComponentID id);

    static void Update();

    static std::vector<size_t> GetActiveBehaviorIDs();

    static array* GetBehaviorInfos();

private:
    static ComponentManager& GetInstance();

    array* m_infos;

    std::bitset<COMPONENT_TABLE_SIZE> m_bitmask;

    std::vector<size_t> m_componentPoolIDs;

    ComponentManager();
    ~ComponentManager();

    void registry(ComponentInfo* info);

    void unregistry(uint64_t hash);

    bool replace(ComponentInfo* dest, const ComponentInfo* src);

    ComponentID allocate(uint64_t hash, ComponentID source);

    void deallocate(uint64_t hash, ComponentID id);

    Behavior* getComponent(uint64_t hash, ComponentID id);

    Behavior* addComponent(uint64_t hash, const Behavior* other);

    bool reinstance(uint64_t hash, Behavior* dest, const Behavior* src);

    void update();

    void resizeComponentInfo();

    template <typename T>
    static void Constructor(Behavior* dest, const Behavior* src);

    template <typename T>
    static void Destructor(Behavior* behavior);
};

template <typename Derived, typename Base, typename Method>
constexpr bool has_override(Method Base::*m)
{
    return &Derived::Update != m; // compara ponteiro de membro
}

template <typename T>
inline ComponentInfo ComponentManager::Registry(const char* name, uint64_t hash)
{
    ComponentInfo info;
    info.size = sizeof(ComponentWrapper<T>);
    info.name = name;
    info.hash = hash;
    info.offset = (size_t)((uint8_t*)&((ComponentWrapper<T>*)0)->component);

    info.constructor = (MethodPointer)&ComponentManager::Constructor<T>;
    info.destructor = (MethodPointer)&ComponentManager::Destructor<T>;

    info.start = (MethodPointer)GetMethodPointer(&T::Start);
    info.update = has_update<T>::value ? (MethodPointer)GetMethodPointer(&T::Update) : 0;

    GetInstance().registry(&info);

    return info;
}

template <typename T>
inline T* ComponentManager::GetComponent(ComponentID id)
{
    return static_cast<T*>(GetInstance().getComponent(GetHashByType<T>("nill"), id));
}

// private
template <typename T>
inline void ComponentManager::Constructor(Behavior* dest, const Behavior* src)
{
    if (src != nullptr)
        new (dest) T(*static_cast<const T*>(src));
    else
        new (dest) T();
}

template <typename T>
inline void ComponentManager::Destructor(Behavior* behavior)
{
    static_cast<T*>(behavior)->~T();
}


