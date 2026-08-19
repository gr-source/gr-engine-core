#pragma once

#include "type_id.hpp"

#include <cstdint>

#define INVALID_SIGNATURE UINT32_MAX

typedef uint64_t Signature;

template <typename... Components>
struct GetComponentMask
{
    static Signature Get() noexcept
    {
        static const Signature mask = sizeof...(Components) > 0 ?
            (0ull | ... | (1ull << GetComponentID<Components>::id)) : INVALID_SIGNATURE;
        return mask;
    }
};

template<typename T>
struct Without
{
    using type = T;
};

template<typename T>
struct IsWithout : std::false_type {};

template<typename T>
struct IsWithout<Without<T>> : std::true_type
{
    using Component = T;
};

template<typename T>
struct GetRequiredMask
{
    static Signature Get()
    {
        if constexpr (IsWithout<T>::value)
            return {};
        else
            return GetComponentMask<T>::Get();
    }
};

template<typename T>
struct GetExcludedMask
{
    static Signature Get()
    {
        if constexpr (IsWithout<T>::value)
            return GetComponentMask<typename IsWithout<T>::Component>::Get();
        else
            return {};
    }
};

template <typename... Components>
using Read = GetComponentMask<Components...>;

template <typename... Components>
using Write = GetComponentMask<Components...>;






