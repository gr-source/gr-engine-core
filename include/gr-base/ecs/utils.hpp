#pragma once

#include "default.hpp"

#include "gr-base/externals/utils.h"

template <typename T>
inline ComponentHash GetHashByType(const char* key = nullptr) noexcept
{
    static ComponentHash hash = fnv1_hash(key);
    return hash;
}

template <typename T>
struct ComponentWrapper
{
    ComponentID next;
    T component;
};

template <typename T>
inline void StartWrapper(Component* obj)
{
    static_cast<T*>(obj)->Start();
}

template <typename T>
inline void UpdateWrapper(Component* obj)
{
    static_cast<T*>(obj)->Update();
}

#include <type_traits>

template <typename T>
struct has_start
{
    static constexpr bool value =
        std::is_same<decltype(&T::Start), void (T::*)()>::value &&
        !std::is_same<decltype(&T::Start), void (Component::*)()>::value;
};

template <typename T>
struct has_update
{
    static constexpr bool value =
        std::is_same<decltype(&T::Update), void (T::*)()>::value &&
        !std::is_same<decltype(&T::Update), void (Component::*)()>::value;
};

template <typename T>
struct has_destroy
{
    static constexpr bool value =
        std::is_same<decltype(&T::Destroy), void (T::*)()>::value &&
        !std::is_same<decltype(&T::Destroy), void (Component::*)()>::value;
};



