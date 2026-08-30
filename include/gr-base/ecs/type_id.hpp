#pragma once

#include "type_traits.hpp"

#include <cstdint>

#ifdef DSCRIPT_BUILD
#define CONSTANT const
#else
#define CONSTANT
#endif

enum class IdCategory
{
    COMPONENT,
    SYSTEM
};

template <IdCategory category>
struct TypeId
{
    static uint32_t Generate(Hash64 hash) noexcept;

    template <typename T>
    static const uint32_t GetID() noexcept
    {
        static const uint32_t id =
            Generate(engine::type_traits<T>::hash);
        return id;
    }
};

template <typename T>
struct GetComponentID
{
    static CONSTANT uint32_t id;
};

#ifdef DSCRIPT_BUILD
template <typename T>
const uint32_t GetComponentID<T>::id =
    TypeId<IdCategory::COMPONENT>::GetID<T>();
#else
template <typename T>
uint32_t GetComponentID<T>::id = 0;
#endif

template <typename T>
struct GetSystemID
{
    static CONSTANT uint32_t id;
};

#ifdef DSCRIPT_BUILD
template <typename T>
const uint32_t GetSystemID<T>::id =
    TypeId<IdCategory::SYSTEM>::GetID<T>();
#else
template <typename T>
uint32_t GetSystemID<T>::id = 0;
#endif


