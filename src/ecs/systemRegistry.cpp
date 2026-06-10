#include "gr-base/ecs/systemRegistry.hpp"

#include "gr-base/core/logger.hpp"
#include <cstdlib>

SystemRegistry* SystemRegistry::s_SystemRegistry = nullptr;

#include "gr-base/system/default.hpp"

void SystemRegistry::Initialize()
{
    if (s_SystemRegistry == nullptr)
    {
        s_SystemRegistry = new SystemRegistry();

        RegistryDefaultSystem();
    }
}

void SystemRegistry::Shutdown()
{
    if (s_SystemRegistry != nullptr)
        delete s_SystemRegistry;
    s_SystemRegistry = nullptr;
}

std::vector<TypeID> SystemRegistry::GetTypesOfMask(Signature mask)
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
SystemRegistry::SystemRegistry() :
    m_capacity(0),
    m_count(0),
    m_systems(nullptr),
    m_nextID(0)
{
    logger::info("[ SystemRegistry ] Initialize '%p'.", this);
}

SystemRegistry::~SystemRegistry()
{
    logger::info("[ SystemRegistry ] Shutdown '%p'.", this);
}

void SystemRegistry::register_info(SystemInfo* src)
{
    if (m_hashMap.find(src->hash) != m_hashMap.end())
        return logger::warn("[ SystemRegistry ] type already registered '%s'.", src->name);

    if (m_count >= m_capacity)
        grow();

    TypeID typeID = static_cast<TypeID>(m_nextID++);
    if (typeID >= m_typeIDToIndex.size())
        m_typeIDToIndex.resize(typeID + 1, INVALID_TYPE);

    TypeID index = static_cast<TypeID>(m_count++);
    m_typeIDToIndex[typeID] = index;
    m_indexToTypeID[index] = typeID;

    auto dst = &m_systems[index];
    memcpy((void*)dst, (void*)src, sizeof(SystemInfo));

    dst->index = typeID;

    m_hashMap.emplace(dst->hash, typeID);

    logger::debug("[ SystemRegistry ] registered: name=%s, hash=0x%llX, index=%d, stride=%ld alignment=%ld",
        dst->name, dst->hash, dst->index, dst->stride, dst->alignment);
}

#include <iostream>
void SystemRegistry::unregister(TypeID id)
{
    if (id >= m_typeIDToIndex.size())
        return;

    TypeID index = m_typeIDToIndex[id];
    if (index == INVALID_TYPE)
        return;

    SystemInfo info = m_systems[index];

    m_typeIDToIndex[id] = INVALID_TYPE;
    m_hashMap.erase(info.hash);

    TypeID lastIndex = static_cast<TypeID>(--m_count);
    if (index != lastIndex)
    {
        auto dst = m_systems + index;
        auto src = m_systems + lastIndex;

        TypeID movedTypeID = m_indexToTypeID[lastIndex];

        memcpy(dst, src, sizeof(ComponentInfo));

        m_typeIDToIndex[movedTypeID] = index;
        m_indexToTypeID[index] = movedTypeID;

        std::cout << "Swap" << std::endl;
    }

    logger::debug("[ SystemRegistry ] unregistered: name=%s, hash=0x%llX, index=%d, stride=%ld alignment=%ld",
        info.name, info.hash, info.index, info.stride, info.alignment);
}

void SystemRegistry::grow()
{
    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_systems = (SystemInfo*)realloc(m_systems, m_capacity * sizeof(SystemInfo));

    m_indexToTypeID.resize(m_capacity, INVALID_TYPE);
}

const SystemInfo* SystemRegistry::getSystemByHash(Hash64 hash) const
{
    auto it = m_hashMap.find(hash);
    if (it == m_hashMap.end())
        return nullptr;
    return getSystemByID(it->second);
}

const SystemInfo* SystemRegistry::getSystemByID(TypeID id) const
{
    if (id >= m_typeIDToIndex.size())
        return nullptr;

    TypeID index = m_typeIDToIndex[id];
    if (index == INVALID_TYPE)
        return nullptr;
    return &m_systems[index];
}

TypeID SystemRegistry::getTypeIDByHash(Hash64 hash) const
{
    auto it = m_hashMap.find(hash);
    if (it != m_hashMap.end())
        return it->second;
    return INVALID_TYPE;
}


