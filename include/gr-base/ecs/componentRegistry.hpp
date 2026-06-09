#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/typeinfo.hpp"

#include "gr-base/externals/hash.hpp"

#include <cstring>
#include <unordered_map>
#include <type_traits>
#include <vector>

class ComponentRegistry
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename T>
    static void Register(const char* name, bool serializable);

    inline static const ComponentInfo* GetComponentInfoByHash(Hash64 hash)
    {
        return s_ComponentRegistry->getComponentInfoByHash(hash);
    }

    inline static const ComponentInfo* GetComponentInfoByName(const char* name)
    {
        return s_ComponentRegistry->getComponentInfoByHash(hash::fnv1_str(name));
    }

    template <typename T>
    static const ComponentInfo* GetType();

    static TypeID GetTypeIDByHash(Hash64 hash)
    {
        return s_ComponentRegistry->getTypeIDByHash(hash);
    }

    inline static TypeID GetTypeIDByName(const char* name)
    {
        return s_ComponentRegistry->getTypeIDByHash(hash::fnv1_str(name));
    }

    template <typename T>
    static TypeID GetTypeID();

    inline static const ComponentInfo* GetComponentInfoByID(TypeID id)
    {
        return s_ComponentRegistry->getComponentInfoByID(id);
    }

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

    std::unordered_map<Hash64, TypeID> m_hashMap;

    ComponentInfo* m_components;
    
    size_t m_capacity;

    size_t m_count;

    std::vector<Hash64> m_typeIDToHash;

    ComponentRegistry();
    ~ComponentRegistry();

    void register_info(ComponentInfo* info);

    void grow();

    const ComponentInfo* getComponentInfoByHash(Hash64 hash) const;

    TypeID getTypeIDByHash(Hash64 hash) const;

    const ComponentInfo* getComponentInfoByID(TypeID id);

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
    static_assert(type_traits<T>::category == TypeCategory::Component,
                  "TypeTraits must be specialized for T");

    ComponentInfo info;
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
    info.constructor = (MethodPointer)ComponentRegistry::Constructor<T>;
    info.destructor = (MethodPointer)ComponentRegistry::Destructor<T>;

    info.copy = (MethodPointer)ComponentRegistry::Copy<T>;
    info.move = (MethodPointer)ComponentRegistry::Move<T>;

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

    s_ComponentRegistry->register_info(&info);
}

template <typename T>
inline const ComponentInfo* ComponentRegistry::GetType()
{
    static ComponentInfo* info = s_ComponentRegistry->getTypeIDByHash(GetHash64ByType<T>());
    return info;
}

template <typename T>
inline TypeID ComponentRegistry::GetTypeID()
{
    static TypeID id = s_ComponentRegistry->getTypeIDByHash(GetHash64ByType<T>());
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



