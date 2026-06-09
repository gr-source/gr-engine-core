#pragma once

#include "gr-base/ecs/typeinfo.hpp"

#include "gr-base/externals/hash.hpp"

#include <cstring>
#include <unordered_map>
#include <type_traits>
#include <vector>

class TypeRegistry
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename T>
    static void Registry(const char* name, bool serializable = false);

    inline static const TypeInfo* GetTypeInfoByHash(Hash64 hash)
    {
        return s_TypeRegistry->getTypeInfoByHash(hash);
    }

    inline static const TypeInfo* GetTypeInfoByName(const char* name)
    {
        return s_TypeRegistry->getTypeInfoByHash(hash::fnv1_str(name));
    }

    template <typename T>
    static const TypeInfo* GetType();

    static TypeID GetTypeIDByHash(Hash64 hash)
    {
        return s_TypeRegistry->getTypeIDByHash(hash);
    }

    inline static TypeID GetTypeIDByName(const char* name)
    {
        return s_TypeRegistry->getTypeIDByHash(hash::fnv1_str(name));
    }

    template <typename T>
    static TypeID GetTypeID();

    inline static const TypeInfo* GetTypeInfoByID(TypeID id)
    {
        return s_TypeRegistry->getTypeInfoByID(id);
    }

    template <typename... Components>
    inline static Signature GetMask()
    {
        static Signature mask = (0ull | ... | (1ull << GetTypeID<Components>()));
        return mask;
    }

    static std::vector<TypeID> GetTypesOfMask(Signature mask);

    inline static const TypeInfo* GetTypeInfoPool()
    {
        return s_TypeRegistry->m_typeInfoPool;
    }

    inline static size_t GetTypeCount()
    {
        return s_TypeRegistry->m_count;
    }

private:
    static TypeRegistry* s_TypeRegistry;

    std::unordered_map<Hash64, TypeID> m_hashToTypeIDMap;

    TypeInfo* m_typeInfoPool;

    std::vector<Hash64> m_typeIDToHash;

    size_t m_capacity;
    size_t m_count;

    TypeRegistry();
    ~TypeRegistry();

    void registry(TypeInfo* info);

    void resize();

    const TypeInfo* getTypeInfoByHash(Hash64 hash) const;

    TypeID getTypeIDByHash(Hash64 hash) const;

    const TypeInfo* getTypeInfoByID(TypeID id);

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
inline void TypeRegistry::Registry(const char* name, bool serializable)
{
    TypeInfo info;
    info.name = name;
    info.hash = GetHash64ByType<T>(name);
    info.index = INVALID_TYPE;
    info.stride = sizeof(T);
    info.alignment = alignof(T);
    info.storage = type_traits<T>::storage;

    // flags
    info.globalCpy = std::is_trivially_copyable_v<T>;
    info.serializable = serializable;

    // Methods
    info.constructor = (MethodPointer)TypeRegistry::Constructor<T>;
    info.destructor = (MethodPointer)TypeRegistry::Destructor<T>;
    info.copy = (MethodPointer)TypeRegistry::Copy<T>;
    info.move = (MethodPointer)TypeRegistry::Move<T>;

    if constexpr (HasSerialize<T>::value)
    {
        info.serialize = (MethodPointer)T::Serialize;
    } else
    {
        info.serialize = nullptr;
    }

    if constexpr (HasDeserialize<T>::value)
    {
        info.deserialize = (MethodPointer)T::Deserialize;
    } else
    {
        info.deserialize = nullptr;
    }

    s_TypeRegistry->registry(&info);
}

template <typename T>
inline const TypeInfo* TypeRegistry::GetType()
{
    static TypeInfo* info = s_TypeRegistry->getTypeIDByHash(GetHash64ByType<T>());
    return info;
}

template <typename T>
inline TypeID TypeRegistry::GetTypeID()
{
    static TypeID id = s_TypeRegistry->getTypeIDByHash(GetHash64ByType<T>());
    return id;
}

template <typename T>
inline void TypeRegistry::Constructor(void* dst, const void* src)
{
    if (src != nullptr)
        new (dst) T(*static_cast<const T*>(src));
    else
        new (dst) T();
}

template <typename T>
inline void TypeRegistry::Destructor(void* dst)
{
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        static_cast<T*>(dst)->~T();
    }
}

template<typename T>
inline void TypeRegistry::Move(void* dst, void* src)
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
inline void TypeRegistry::Copy(void* dst, const void* src)
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



