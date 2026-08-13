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

template <typename... Components>
using Read = GetComponentMask<Components...>;

template <typename... Components>
using Write = GetComponentMask<Components...>;






