#pragma once

#include "gr-base/pool/sparseSet.hpp"

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/typeinfo.hpp"

#include "gr-base/externals/hash.hpp"

#include <unordered_map>

class ComponentRegistry
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename T>
    static void Register(bool serializable);

    template <typename T>
    static void Unregister();

    inline static const ComponentInfo* GetComponentInfoByHash(Hash64 hash)
    {
        return s_ComponentRegistry->getComponentInfoByHash(hash);
    }

    inline static const ComponentInfo* GetComponentInfoByName(const char* name)
    {
        return s_ComponentRegistry->getComponentInfoByHash(engine::hash::fnv1_str(name));
    }

    template <typename T>
    static const ComponentInfo* GetType();

    static TypeID GetTypeIDByHash(Hash64 hash)
    {
        return s_ComponentRegistry->getTypeIDByHash(hash);
    }

    static TypeID GetTypeIDByName(const char* name);

    template <typename T>
    static TypeID GetTypeID();

    static const ComponentInfo* GetComponentByID(TypeID id);

    inline static const ComponentInfo* GetComponents()
    {
        return s_ComponentRegistry->sparseSet.data();
    }

    inline static size_t GetComponentCount()
    {
        return s_ComponentRegistry->sparseSet.count();
    }

private:
    static ComponentRegistry* s_ComponentRegistry;

    std::unordered_map<Hash64, TypeID> m_persistentTypeIDs;

    std::unordered_map<Hash64, TypeID> m_registeredTypeIDs;
    
    SparseSet<ComponentInfo> sparseSet;

    size_t m_nextID =
        0;

    ComponentRegistry();
    ~ComponentRegistry();

    void register_info(ComponentInfo* info);

    void unregister(TypeID id);

    const ComponentInfo* getComponentInfoByHash(Hash64 hash) const;

    TypeID getTypeIDByHash(Hash64 hash) const;

    const ComponentInfo* getComponentByID(TypeID id) const;

    template <typename T>
    static void Constructor(void* dest, const void* src);

    template <typename T>
    static void Destructor(void* dest);

    template <typename T>
    static void Copy(void* dst, const void* src);

    template <typename T>
    static void Move(void* dst, void* src);
};

template <typename T>
inline void ComponentRegistry::Register(bool serializable)
{
    static_assert(engine::type_traits<T>::size > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::category_traits<T>::value == TypeCategory::Component,
        "TypeTraits must be specialized for T");

    std::string_view temp = engine::type_traits<T>::name;

    ComponentInfo info;

    strncpy(info.name, temp.data(), sizeof(info.name));
    info.name[temp.size()] = '\0';

    info.hash = engine::type_traits<T>::hash;
    info.index = INVALID_TYPE;

    info.stride = std::is_empty_v<T> ? 0 : engine::type_traits<T>::size;
    info.alignment = std::is_empty_v<T> ? 0 : engine::type_traits<T>::alignment;
    
    info.storage = engine::storage_traits<T>::value;

    // flags
    info.isTriviallyCopyable = std::is_trivially_copyable_v<T>;
    info.serializable = serializable;

    // Methods
    info.constructor = &ComponentRegistry::Constructor<T>;
    info.destructor = &ComponentRegistry::Destructor<T>;

    info.copy = &ComponentRegistry::Copy<T>;
    info.move = &ComponentRegistry::Move<T>;

    if constexpr (engine::serialize<T>::value)
    {
        info.serialize = (MethodPointer)T::Serialize;
    } else
    {
        info.serialize = nullptr;
    }

    if constexpr (engine::deserialize<T>::value)
    {
        info.deserialize = (MethodPointer)T::Deserialize;
    } else
    {
        info.deserialize = nullptr;
    }

    s_ComponentRegistry->register_info(&info);
}

template <typename T>
inline void ComponentRegistry::Unregister()
{
    static_assert(engine::type_traits<T>::size > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::category_traits<T>::value == TypeCategory::Component,
        "TypeTraits must be specialized for T");

    TypeID id = s_ComponentRegistry->getTypeIDByHash(engine::type_traits<T>::hash);
    if (id != INVALID_TYPE)
        s_ComponentRegistry->unregister(id);
}

template <typename T>
inline const ComponentInfo* ComponentRegistry::GetType()
{
    static_assert(engine::type_traits<T>::size > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::category_traits<T>::value == TypeCategory::Component,
        "TypeTraits must be specialized for T");

    static ComponentInfo* info = s_ComponentRegistry->getTypeIDByHash(engine::type_traits<T>::hash);
    return info;
}

template <typename T>
inline TypeID ComponentRegistry::GetTypeID()
{
    static_assert(engine::type_traits<T>::size > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::category_traits<T>::value == TypeCategory::Component,
        "TypeTraits must be specialized for T");

    static TypeID cacheID = s_ComponentRegistry->getTypeIDByHash(engine::type_traits<T>::hash);
    return cacheID;
}

template <typename T>
inline void ComponentRegistry::Constructor(void* dst, const void* src)
{
    if (src != nullptr)
        new (dst) T(*static_cast<const T*>(src));
    else
        new (dst) T();
}

template <typename T>
inline void ComponentRegistry::Destructor(void* dst)
{
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        static_cast<T*>(dst)->~T();
    }
}

template<typename T>
inline void ComponentRegistry::Move(void* dst, void* src)
{
    if constexpr (std::is_trivially_copyable_v<T>)
    {
        memcpy(dst, src, sizeof(T));
    }
    else
    {
        new (dst) T(std::move(*static_cast<T*>(src)));
    }
}

template <typename T>
inline void ComponentRegistry::Copy(void* dst, const void* src)
{
    if constexpr (std::is_trivially_copyable_v<T>)
    {
        memcpy(dst, src, sizeof(T));
    }
    else
    {
        new(dst) T(*static_cast<const T*>(src));
    }
}



