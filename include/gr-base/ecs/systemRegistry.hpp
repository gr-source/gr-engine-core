#pragma once

#include "gr-base/ecs/typeinfo.hpp"
#include "gr-base/ecs/default.hpp"

#include <unordered_map>
#include <cstring>
#include <vector>

class SystemRegistry
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename T, typename... Systems>
    static void Register();

    template <typename T>
    static void Unregister();

    inline static const SystemInfo* GetSystemByHash(Hash64 hash)
    {
        return s_SystemRegistry->getSystemByHash(hash);
    }

    inline static const SystemInfo* GetSystemByName(const char* name)
    {
        return s_SystemRegistry->getSystemByHash(engine::hash::fnv1_str(name));
    }

    template <typename T>
    static const SystemInfo* GetSystem();

    static TypeID GetTypeIDByHash(Hash64 hash)
    {
        return s_SystemRegistry->getTypeIDByHash(hash);
    }

    inline static TypeID GetTypeIDByName(const char* name)
    {
        return s_SystemRegistry->getTypeIDByHash(engine::hash::fnv1_str(name));
    }

    template <typename T>
    static TypeID GetTypeID();

    inline static const SystemInfo* GetSystemByID(TypeID id)
    {
        return s_SystemRegistry->getSystemByID(id);
    }

    template <typename... Components>
    inline static Signature GetMask()
    {
        static Signature mask = (0ull | ... | (1ull << GetTypeID<Components>()));
        return mask;
    }

    static std::vector<TypeID> GetTypesOfMask(Signature mask);

    inline static size_t GetCount()
    {
        return s_SystemRegistry->m_count;
    }

    inline static const SystemInfo* GetSystems()
    {
        return s_SystemRegistry->m_systems;
    }

private:
    static SystemRegistry* s_SystemRegistry;

    std::unordered_map<Hash64, TypeID> m_cachedID;

    std::unordered_map<Hash64, TypeID> m_hashMap;

    std::vector<TypeID> m_typeIDToIndex;
    std::vector<TypeID> m_indexToTypeID;

    SystemInfo* m_systems;

    size_t m_capacity;

    size_t m_count;

    size_t m_nextID;

    std::vector<Hash64> m_typeIDToHash;

    SystemRegistry();
    ~SystemRegistry();

    void register_info(SystemInfo* info);

    void unregister(TypeID id);

    void grow();

    const SystemInfo* getSystemByHash(Hash64 hash) const;

    const SystemInfo* getSystemByID(TypeID id) const;

    TypeID getTypeIDByHash(Hash64 hash) const;

    template <typename T>
    static void Constructor(void* dest, const void* src);

    template <typename T>
    static void Destructor(void* dest);

    template <typename T>
    static void Copy(void* dst, const void* src);

    template <typename T>
    static void Move(void* dst, void* src);
};

template <typename T, typename... Systems>
inline void SystemRegistry::Register()
{
    static_assert(engine::type_traits::size<T> > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::type_traits::category<T> == TypeCategory::System,
        "TypeTraits must be specialized for T");

    std::string_view temp = engine::type_traits::name<T>;

    SystemInfo info;

    strncpy(info.name, temp.data(), sizeof(info.name));
    info.name[temp.size()] = '\0';

    info.hash = engine::type_traits::hash<T>;
    info.index = INVALID_TYPE;

    info.stride = engine::type_traits::size<T>;
    info.alignment = engine::type_traits::alignment<T>;
    
    // Methods
    info.constructor = &SystemRegistry::Constructor<T>;
    info.destructor = &SystemRegistry::Destructor<T>;

    info.copy = nullptr; // &SystemRegistry::Copy<T>;
    info.move = nullptr; //&SystemRegistry::Move<T>;

    info.depends_on = GetMask<Systems...>();

    s_SystemRegistry->register_info(&info);
}

template <typename T>
inline const SystemInfo* SystemRegistry::GetSystem()
{
    static_assert(engine::type_traits::size<T> > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::type_traits::category<T> == TypeCategory::System,
        "TypeTraits must be specialized for T");

    static const SystemInfo* info = s_SystemRegistry->getSystemByHash(engine::type_traits::hash<T>);
    return info;
}

template <typename T>
inline void SystemRegistry::Unregister()
{
    static_assert(engine::type_traits::size<T> > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::type_traits::category<T> == TypeCategory::System,
        "TypeTraits must be specialized for T");

    TypeID id = s_SystemRegistry->getTypeIDByHash(engine::type_traits::hash<T>);
    if (id != INVALID_TYPE)
        s_SystemRegistry->unregister(id);
}

template <typename T>
inline TypeID SystemRegistry::GetTypeID()
{
    static_assert(engine::type_traits::size<T> > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::type_traits::category<T> == TypeCategory::System,
        "TypeTraits must be specialized for T");

    static TypeID id = s_SystemRegistry->getTypeIDByHash(engine::type_traits::hash<T>);
    return id;
}

template <typename T>
inline void SystemRegistry::Constructor(void* dst, const void* src)
{
    new (dst) T();
}

template <typename T>
inline void SystemRegistry::Destructor(void* dst)
{
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        static_cast<T*>(dst)->~T();
    }
}

template<typename T>
inline void SystemRegistry::Move(void* dst, void* src)
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
inline void SystemRegistry::Copy(void* dst, const void* src)
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



