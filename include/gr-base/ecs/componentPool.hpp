#pragma once

#include "pool.hpp"

#include "ecs/default.hpp"

template <typename T>
class ComponentPool : public Pool {
public:
    ComponentPool();

    Component *CreateInstance(const Component *component, Entity *entity) override;

    Component *Instance(Component *component) override;
};

template <typename T>
inline ComponentPool<T>::ComponentPool() : Pool(sizeof(T), typeid(T).name()) {}

template <typename T>
inline Component *ComponentPool<T>::CreateInstance(const Component *component, Entity *entity) {
    auto ptr = Pool::CreatComponent(sizeof(T), entity);
    if (!ptr) {
        return nullptr;
    }

    ptr = component != nullptr ? new (ptr) T(*static_cast<const T *>(component)) : new (ptr) T();

    return ptr;
}

template <typename T>
inline Component *ComponentPool<T>::Instance(Component *component) {
    return new (component) T(*static_cast<T *>(component));
}

