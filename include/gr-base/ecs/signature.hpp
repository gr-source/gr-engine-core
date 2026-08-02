#pragma once

#include "type_id.hpp"

#include <cstdint>

typedef uint64_t Signature;

template <typename... Components>
struct GetComponentMask
{
    static inline Signature Get() noexcept
    {
        static const Signature mask =
            (0ull | ... | (1ull << GetComponentID<Components>::id));
        return mask;
    }
};







