#pragma once

#include "gr-base/externals/hash.hpp"

#include <type_traits>
#include <string_view>

#define COMPONENT_CHUNK_SIZE (16 * 1024)

class Stream;

enum class TypeCategory
{
    Unknown,
    Component,
    System
};

enum class StorageKind
{
    Archetype,
    Sparse
};

namespace engine
{
template <typename T>
struct category_traits
{
    static constexpr TypeCategory value =
        TypeCategory::Unknown;
};

template <typename T>
struct storage_traits
{
    static constexpr StorageKind value =
        StorageKind::Archetype;
};

template <typename T>
struct type_traits
{
    static constexpr std::string_view get_type_name()
    {
#if defined(__clang__)
        constexpr auto prefix = std::string_view{"[T = "};
        constexpr auto suffix = std::string_view{"]"};
        constexpr auto func = std::string_view{__PRETTY_FUNCTION__};
#else // GCC
        constexpr auto prefix = std::string_view{"with T = "};
        constexpr auto suffix = std::string_view{";"};
        constexpr auto func = std::string_view{__PRETTY_FUNCTION__};
#endif
        constexpr auto start = func.find(prefix) + prefix.size();
        constexpr auto end = func.find(suffix);
        return func.substr(start, end - start);
    }

    static constexpr std::string_view name =
        get_type_name();

    static constexpr Hash64 hash =
        engine::hash::fnv1_str(get_type_name());

    static constexpr bool tag =
        std::is_empty_v<T>;

    static constexpr size_t size =
        sizeof(T);

    static constexpr size_t alignment =
        alignof(T);

    static constexpr size_t stride = sizeof(T);

    static constexpr size_t capacity = (stride > 0) ? (COMPONENT_CHUNK_SIZE / stride) : 0;
};

template<typename, typename = void>
struct serialize : std::false_type {};

template<typename T>
struct serialize<
    T,
    std::void_t<
        decltype(std::declval<T>().serialize(std::declval<Stream*>()))
    >
> : std::bool_constant<std::is_same_v<
        decltype(std::declval<T>().serialize(std::declval<Stream*>())), bool
    >> {};

template<typename, typename = void>
struct deserialize : std::false_type {};

template<typename T>
struct deserialize<
    T,
    std::void_t<
        decltype(std::declval<T>().deserialize(std::declval<Stream*>()))
    >
> : std::bool_constant<std::is_same_v<
        decltype(std::declval<T>().deserialize(std::declval<Stream*>())), bool
    >> {};

} // engine::type_traits

