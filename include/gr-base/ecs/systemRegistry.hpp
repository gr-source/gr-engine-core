#pragma once

#include "gr-base/ecs/typeinfo.hpp"

#include "gr-base/pool/sparseSet.hpp"

#include <unordered_map>

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

    inline static size_t GetCount()
    {
        return s_SystemRegistry->sparseSet.count();
    }

    inline static const SystemInfo* GetSystems()
    {
        return s_SystemRegistry->sparseSet.data();
    }

private:
    static SystemRegistry* s_SystemRegistry;

    std::unordered_map<Hash64, TypeID> m_persistentTypeIDs;

    std::unordered_map<Hash64, TypeID> m_registeredTypeIDs;
    
    SparseSet<SystemInfo> sparseSet;

    size_t m_nextID =
        0;

    SystemRegistry();
    ~SystemRegistry();

    void register_info(SystemInfo* info);

    void unregister(TypeID id);

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
    static_assert(engine::type_traits<T>::size > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::category_traits<T>::value == TypeCategory::System,
        "TypeTraits must be specialized for T");

    std::string_view temp = engine::type_traits<T>::name;

    SystemInfo info;

    strncpy(info.name, temp.data(), sizeof(info.name));
    info.name[temp.size()] = '\0';

    info.hash = engine::type_traits<T>::hash;
    info.index = INVALID_TYPE;

    info.stride = engine::type_traits<T>::size;
    info.alignment = engine::type_traits<T>::alignment;
    
    // Methods
    info.constructor = &SystemRegistry::Constructor<T>;
    info.destructor = &SystemRegistry::Destructor<T>;

    info.copy = nullptr; // &SystemRegistry::Copy<T>;
    info.move = nullptr; //&SystemRegistry::Move<T>;

    s_SystemRegistry->register_info(&info);
}

template <typename T>
inline const SystemInfo* SystemRegistry::GetSystem()
{
    static_assert(engine::type_traits<T>::size > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::category_traits<T>::value == TypeCategory::System,
        "TypeTraits must be specialized for T");

    static const SystemInfo* info = s_SystemRegistry->getSystemByHash(engine::type_traits<T>::hash);
    return info;
}

template <typename T>
inline void SystemRegistry::Unregister()
{
    static_assert(engine::type_traits<T>::size > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::category_traits<T>::value == TypeCategory::System,
        "TypeTraits must be specialized for T");

    TypeID id = s_SystemRegistry->getTypeIDByHash(engine::type_traits<T>::hash);
    if (id != INVALID_TYPE)
        s_SystemRegistry->unregister(id);
}

template <typename T>
inline TypeID SystemRegistry::GetTypeID()
{
    static_assert(engine::type_traits<T>::size > 0,
        "TypeTraits must be a complete type.");
    static_assert(engine::category_traits<T>::value == TypeCategory::System,
        "TypeTraits must be specialized for T");

    static TypeID id = s_SystemRegistry->getTypeIDByHash(engine::type_traits<T>::hash);
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



