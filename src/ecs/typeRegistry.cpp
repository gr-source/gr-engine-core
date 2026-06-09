#include "gr-base/ecs/typeRegistry.hpp"

#include "gr-base/core/logger.hpp"

TypeRegistry* TypeRegistry::s_TypeRegistry = nullptr;

void TypeRegistry::Initialize()
{
    if (s_TypeRegistry == nullptr)
        s_TypeRegistry = new TypeRegistry();
}

void TypeRegistry::Shutdown()
{
    if (s_TypeRegistry != nullptr)
        delete s_TypeRegistry;
    s_TypeRegistry = nullptr;
}

std::vector<TypeID> TypeRegistry::GetTypesOfMask(Signature mask)
{
    std::vector<TypeID> types;
    while (mask)
    {
        types.push_back(__builtin_ctzll(mask));
        
        mask &= (mask - 1);
    }
    return types;
}

// private
TypeRegistry::TypeRegistry() : m_count(0), m_capacity(0), m_typeInfoPool(nullptr)
{
    logger::info("[ TypeRegistry ] Initialize '%p'.", this);
}

TypeRegistry::~TypeRegistry()
{
    logger::info("[ TypeRegistry ] Shutdown '%p'.", this);
}

void TypeRegistry::registry(TypeInfo* src)
{
    if (m_hashToTypeIDMap.find(src->hash) != m_hashToTypeIDMap.end())
        return logger::warn("[Type] TypeRegistry::registry - type already registered '%s'.", src->name);

    if (m_count >= m_capacity)
        resize();

    TypeID id = static_cast<TypeID>(m_count++);

    TypeInfo* dest = m_typeInfoPool + id;
    memcpy((void*)dest, (void*)src, sizeof(TypeInfo));

    dest->index = id;

    m_hashToTypeIDMap.emplace(dest->hash, id);

    logger::debug("[TypeRegistry::registry] registered: name=%s, hash=%llu, index=%d, stride=%ld alignment=%ld", \
        dest->name, dest->hash, dest->index, dest->stride, dest->alignment); \
}

void TypeRegistry::resize()
{
    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_typeInfoPool = (TypeInfo*)realloc(m_typeInfoPool, m_capacity * sizeof(TypeInfo));
}

const TypeInfo* TypeRegistry::getTypeInfoByHash(Hash64 hash) const
{
    auto it = m_hashToTypeIDMap.find(hash);
    if (it != m_hashToTypeIDMap.end())
        return m_typeInfoPool + it->second;
    return nullptr;
}

TypeID TypeRegistry::getTypeIDByHash(Hash64 hash) const
{
    auto typeInfo = getTypeInfoByHash(hash);
    return typeInfo != nullptr ? typeInfo->index : INVALID_TYPE;
}

const TypeInfo* TypeRegistry::getTypeInfoByID(TypeID id)
{
    assert(id < m_count);

    return m_typeInfoPool + id;
}


