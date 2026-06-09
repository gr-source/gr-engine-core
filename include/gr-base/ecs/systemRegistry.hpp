#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/typeinfo.hpp"

#include "gr-base/externals/hash.hpp"

#include <cstring>
#include <unordered_map>
#include <type_traits>
#include <vector>

class SystemRegistry
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename T, typename... Systems>
    static void Register(const char* name);

    inline static const SystemInfo* GetSystemInfoByHash(Hash64 hash)
    {
        return s_SystemRegistry->getSystemInfoByHash(hash);
    }

    inline static const SystemInfo* GetSystemInfoByName(const char* name)
    {
        return s_SystemRegistry->getSystemInfoByHash(hash::fnv1_str(name));
    }

    template <typename T>
    static const SystemInfo* GetType();

    static TypeID GetTypeIDByHash(Hash64 hash)
    {
        return s_SystemRegistry->getTypeIDByHash(hash);
    }

    inline static TypeID GetTypeIDByName(const char* name)
    {
        return s_SystemRegistry->getTypeIDByHash(hash::fnv1_str(name));
    }

    template <typename T>
    static TypeID GetTypeID();

    inline static const SystemInfo* GetSystemInfoByID(TypeID id)
    {
        return s_SystemRegistry->getSystemInfoByID(id);
    }

    template <typename... Components>
    inline static Signature GetMask()
    {
        static Signature mask = (0ull | ... | (1ull << GetTypeID<Components>()));
        return mask;
    }

    static std::vector<TypeID> GetTypesOfMask(Signature mask);

private:
    static SystemRegistry* s_SystemRegistry;

    std::unordered_map<Hash64, TypeID> m_hashMap;

    SystemInfo* m_systems;
    size_t m_capacity;
    size_t m_count;

    std::vector<Hash64> m_typeIDToHash;

    SystemRegistry();
    ~SystemRegistry();

    void register_info(SystemInfo* info);

    void grow();

    const SystemInfo* getSystemInfoByHash(Hash64 hash) const;

    TypeID getTypeIDByHash(Hash64 hash) const;

    const SystemInfo* getSystemInfoByID(TypeID id);

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
inline void SystemRegistry::Register(const char *name)
{
    static_assert(type_traits<T>::category == TypeCategory::System,
                  "TypeTraits must be specialized for T");

    SystemInfo info;
    info.name = name;
    info.hash = GetHash64ByType<T>(name);
    info.index = INVALID_TYPE;

    info.stride = sizeof(T);
    info.alignment = alignof(T);
    
    // Methods
    info.constructor = (MethodPointer)SystemRegistry::Constructor<T>;
    info.destructor = (MethodPointer)SystemRegistry::Destructor<T>;

    info.copy = (MethodPointer)SystemRegistry::Copy<T>;
    info.move = (MethodPointer)SystemRegistry::Move<T>;

    info.depends_on = GetMask<Systems...>();

    s_SystemRegistry->register_info(&info);
}

template <typename T>
inline const SystemInfo* SystemRegistry::GetType()
{
    static SystemInfo* info = s_SystemRegistry->getTypeIDByHash(GetHash64ByType<T>());
    return info;
}

template <typename T>
inline TypeID SystemRegistry::GetTypeID()
{
    static TypeID id = s_SystemRegistry->getTypeIDByHash(GetHash64ByType<T>());
    return id;
}

template <typename T>
inline void SystemRegistry::Constructor(void* dst, const void* src)
{
    if (src != nullptr)
        new (dst) T(*static_cast<const T*>(src));
    else
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



