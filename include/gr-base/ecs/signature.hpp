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

template <typename T>
struct RequiredMask
{
    static Signature Get() noexcept
    {
        if constexpr (IsWithout<T>::value)
        {
            return 0;
        }
        else
        {
            return 1ull << GetComponentID<T>::id;
        }
    }
};

template <typename T>
struct ExcludedMask
{
    static Signature Get() noexcept
    {
        if constexpr (IsWithout<T>::value)
        {
            using Component = typename IsWithout<T>::Component;

            return 1ull << GetComponentID<T>::id;
        }
        else
        {
            return 0;
        }
    }
};

template <typename... Components>
struct Query
{
    static Signature GetRequiredMask() noexcept
    {
        return (0ull | ... | RequiredMask<Components>::Get());
    }

    static Signature GetExcludedMask() noexcept
    {
        return (0ull | ... | ExcludedMask<Components>::Get());
    }
};

template <typename... Components>
using Read = GetComponentMask<Components...>;

template <typename... Components>
using Write = GetComponentMask<Components...>;






