#pragma once

#include "default.hpp"

inline constexpr uint64_t GetHash(const char* str) noexcept
{
    uint64_t hash = 14695981039346656037UL;

    char c = '\0';
    while ((c = *str++))
    {
        hash ^= (uint64_t)(unsigned char)(c);
        hash *= 1099511628211UL;
    }
    return hash;
}


template <typename T>
inline uint64_t GetHashByType(const char* key) noexcept
{
    static uint64_t hash = GetHash(key);
    return hash;
}

template <typename T>
struct ComponentWrapper
{
    ComponentID next;
    T component;
};

template <typename T>
inline void StartWrapper(Behavior* obj)
{
    static_cast<T*>(obj)->Start();
}

template <typename T>
inline void UpdateWrapper(Behavior* obj)
{
    static_cast<T*>(obj)->Update();
}

#include <type_traits>

template <typename T>
struct has_start
{
    static constexpr bool value =
        std::is_same<decltype(&T::Start), void (T::*)()>::value &&
        !std::is_same<decltype(&T::Start), void (Behavior::*)()>::value;
};

template <typename T>
struct has_update
{
    static constexpr bool value =
        std::is_same<decltype(&T::Update), void (T::*)()>::value &&
        !std::is_same<decltype(&T::Update), void (Behavior::*)()>::value;
};

template <typename T>
struct has_destroy
{
    static constexpr bool value =
        std::is_same<decltype(&T::Destroy), void (T::*)()>::value &&
        !std::is_same<decltype(&T::Destroy), void (Behavior::*)()>::value;
};



