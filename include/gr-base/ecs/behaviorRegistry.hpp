#pragma once

#include "componentinfo.hpp"
#include "utils.hpp"

#include <cstddef>
#include <vector>
#include <bitset>

class BehaviorRegistry
{
public:
    template <typename T>
    static ComponentInfo Registry(const char* name, ComponentHash hash, ComponentHash parent);

    static void Unregistry(ComponentHash hash);

    static ComponentInfo* Get(ComponentHash hash);

    static std::vector<ComponentHash> GetActiveBehaviorActive();

    static array* GetBehaviorInfos();

private:
    static BehaviorRegistry& GetInstance();

    std::bitset<COMPONENT_TABLE_SIZE> m_bitmask;

    std::vector<ComponentHash> m_behaviorActives;

    array* m_infos;

    BehaviorRegistry();
    ~BehaviorRegistry();

    void registry(ComponentInfo* info);

    void unregistry(ComponentHash hash);

    ComponentInfo* get(ComponentHash hash);

    bool reconfigure(ComponentInfo* dest, const ComponentInfo* src);

    void resizeComponentInfo();

    template <typename T>
    static void Constructor(Behavior* dest, const Behavior* src);

    template <typename T>
    static void Destructor(Behavior* behavior);
};

template <typename T>
inline ComponentInfo BehaviorRegistry::Registry(const char* name, ComponentHash hash, ComponentHash parent)
{
    ComponentInfo info;
    info.stride = sizeof(T);
    info.stride = sizeof(struct ComponentWrapper<T>);
    info.name = name;
    info.hash = hash;
    info.parent = parent;
    info.offset = (size_t)((uint8_t*)&((ComponentWrapper<T>*)0)->component);

    info.constructor = (MethodPointer)BehaviorRegistry::Constructor<T>;
    info.destructor = (MethodPointer)BehaviorRegistry::Destructor<T>;

    info.start = has_start<T>::value ? (MethodPointer)StartWrapper<T> : 0;
    info.update = has_update<T>::value ? (MethodPointer)UpdateWrapper<T> : 0;
    // info.destroy = has_destroy<T>::value ? (MethodPointer)GetMethodPointer(&T::Destroy) : 0;

    GetInstance().registry(&info);

    return info;
}

// private
template <typename T>
inline void BehaviorRegistry::Constructor(Behavior* dest, const Behavior* src)
{
    if (src != nullptr)
        new (dest) T(*static_cast<const T*>(src));
    else
        new (dest) T();
}

template <typename T>
inline void BehaviorRegistry::Destructor(Behavior* behavior)
{
    static_cast<T*>(behavior)->~T();
}



