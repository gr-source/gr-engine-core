#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/typeinfo.hpp"

#include "gr-base/externals/hash.hpp"

#include <unordered_map>
#include <cstring>
#include <vector>

class ComponentRegistry
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename T>
    static void Register(const char* name, bool serializable);

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

    inline static TypeID GetTypeIDByName(const char* name)
    {
        return s_ComponentRegistry->getTypeIDByHash(engine::hash::fnv1_str(name));
    }

    template <typename T>
    static TypeID GetTypeID();

    static const ComponentInfo* GetComponentByID(TypeID id);

    template <typename... Components>
    inline static Signature GetMask()
    {
        static Signature mask = (0ull | ... | (1ull << GetTypeID<Components>()));
        return mask;
    }

    static std::vector<TypeID> GetTypesOfMask(Signature mask);

    inline static const ComponentInfo* GetComponents()
    {
        return s_ComponentRegistry->m_components;
    }

    inline static size_t GetComponentCount()
    {
        return s_ComponentRegistry->m_count;
    }

private:
    static ComponentRegistry* s_ComponentRegistry;

    std::unordered_map<Hash64, TypeID> m_bitsetMap;

    std::unordered_map<Hash64, TypeID> m_hashMap;
    
    std::vector<Hash64> m_typeIDToHash;

    std::vector<TypeID> m_typeIDToIndex;
    std::vector<TypeID> m_indexToTypeID;

    ComponentInfo* m_components;
    
    size_t m_capacity;

    size_t m_count;

    size_t m_nextID;

    ComponentRegistry();
    ~ComponentRegistry();

    void register_info(ComponentInfo* info);

    void unregister(TypeID id);

    void grow();

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
inline void ComponentRegistry::Register(const char* name, bool serializable)
{
    static_assert(engine::type_traits::size<T> > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::type_traits::category<T> == TypeCategory::Component,
        "TypeTraits must be specialized for T");

    ComponentInfo info;
    info.name = name;
    info.hash = engine::type_traits::hash<T>;
    info.index = INVALID_TYPE;

    info.stride = engine::type_traits::size<T>;
    info.alignment = engine::type_traits::alignment<T>;
    
    info.storage = engine::type_traits::storage<T>;

    // flags
    info.globalCpy = std::is_trivially_copyable_v<T>;
    info.serializable = serializable;

    // Methods
    info.constructor = (MethodPointer)ComponentRegistry::Constructor<T>;
    info.destructor = (MethodPointer)ComponentRegistry::Destructor<T>;

    info.copy = (MethodPointer)ComponentRegistry::Copy<T>;
    info.move = (MethodPointer)ComponentRegistry::Move<T>;

    if constexpr (engine::type_traits::serialize<T>::value)
    {
        info.serialize = (MethodPointer)T::Serialize;
    } else
    {
        info.serialize = nullptr;
    }

    if constexpr (engine::type_traits::deserialize<T>::value)
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
    static_assert(engine::type_traits::size<T> > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::type_traits::category<T> == TypeCategory::Component,
        "TypeTraits must be specialized for T");

    TypeID id = s_ComponentRegistry->getTypeIDByHash(engine::type_traits::hash<T>);
    if (id != INVALID_TYPE)
        s_ComponentRegistry->unregister(id);
}

template <typename T>
inline const ComponentInfo* ComponentRegistry::GetType()
{
    static_assert(engine::type_traits::size<T> > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::type_traits::category<T> == TypeCategory::Component,
        "TypeTraits must be specialized for T");

    static ComponentInfo* info = s_ComponentRegistry->getTypeIDByHash(engine::type_traits::hash<T>);
    return info;
}

template <typename T>
inline TypeID ComponentRegistry::GetTypeID()
{
    static_assert(engine::type_traits::size<T> > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::type_traits::category<T> == TypeCategory::Component,
        "TypeTraits must be specialized for T");

    static TypeID id = s_ComponentRegistry->getTypeIDByHash(engine::type_traits::hash<T>);
    return id;
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



