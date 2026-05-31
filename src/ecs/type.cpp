#include "gr-base/ecs/type.hpp"
#include "gr-base/core/logger.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>

// private
Type& Type::GetInstance()
{
    static Type s_Type;
    return s_Type;
}

Type::Type() : m_count(0), m_capacity(0), m_typeInfoPool(nullptr)
{
    logger::info("[Type] Initialize");
}

Type::~Type()
{
    logger::info("[Type] Shutdown");
}

void Type::registry(TypeInfo* src)
{
    if (m_hashToTypeIDMap.find(src->hash) != m_hashToTypeIDMap.end())
        return logger::warn("[Type] Type::registry - type already registered '%s'.", src->name);

    if (m_count >= m_capacity)
        resize();

    TypeID id = m_count++;

    TypeInfo* dest = m_typeInfoPool + id;
    memcpy((void*)dest, (void*)src, sizeof(TypeInfo));

    dest->index = id;

    m_hashToTypeIDMap.emplace(dest->hash, id);

    logger::debug("[Type::registry] registered: name=%s, hash=%llu, index=%d, stride=%ld alignment=%ld", \
        dest->name, dest->hash, dest->index, dest->stride, dest->alignment); \
}

void Type::resize()
{
    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_typeInfoPool = (TypeInfo*)realloc(m_typeInfoPool, m_capacity * sizeof(TypeInfo));
}

const TypeInfo* Type::getTypeInfoByHash(Hash64 hash) const
{
    auto it = m_hashToTypeIDMap.find(hash);
    if (it != m_hashToTypeIDMap.end())
        return m_typeInfoPool + it->second;
    return nullptr;
}

TypeID Type::getTypeIDByHash(Hash64 hash) const
{
    auto typeInfo = getTypeInfoByHash(hash);
    return typeInfo != nullptr ? typeInfo->index : INVALID_TYPE;
}

const TypeInfo* Type::getTypeInfoByID(TypeID id)
{
    assert(id < m_count);

    return m_typeInfoPool + id;
}


