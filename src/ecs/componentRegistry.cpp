#include "gr-base/ecs/componentRegistry.hpp"

#include "gr-base/core/logger.hpp"
#include <cstdlib>

ComponentRegistry* ComponentRegistry::s_ComponentRegistry = nullptr;

#include "gr-base/script/default.hpp"

void ComponentRegistry::Initialize()
{
    if (s_ComponentRegistry == nullptr)
    {
        s_ComponentRegistry = new ComponentRegistry();

        RegistryDefaultComponent();
    }
}

void ComponentRegistry::Shutdown()
{
    if (s_ComponentRegistry != nullptr)
        delete s_ComponentRegistry;
    s_ComponentRegistry = nullptr;
}

const ComponentInfo* ComponentRegistry::GetComponentByID(TypeID id)
{
    return s_ComponentRegistry->getComponentByID(id);
}

std::vector<TypeID> ComponentRegistry::GetTypesOfMask(Signature mask)
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
ComponentRegistry::ComponentRegistry() :
    m_capacity(0),
    m_count(0),
    m_components(nullptr),
    m_nextID(0)
{
    logger::info("[ ComponentRegistry ] Initialize '%p'.", this);
}

ComponentRegistry::~ComponentRegistry()
{
    logger::info("[ ComponentRegistry ] Shutdown '%p'.", this);

    if (m_components != nullptr)
        free(m_components);
}

void ComponentRegistry::register_info(ComponentInfo* src)
{
    if (m_hashMap.find(src->hash) != m_hashMap.end())
        return logger::warn("[ ComponentRegistry ] type already registered '%s'.", src->name);

    if (m_count >= m_capacity)
        grow();

    if (m_bitsetMap.find(src->hash) == m_bitsetMap.end())
        m_bitsetMap.emplace(src->hash, static_cast<TypeID>(m_nextID++));

    TypeID typeID = m_bitsetMap[src->hash];
    if (typeID >= m_typeIDToIndex.size())
        m_typeIDToIndex.resize(typeID + 1, INVALID_TYPE);

    TypeID index = static_cast<TypeID>(m_count++);
    m_typeIDToIndex[typeID] = index;
    m_indexToTypeID[index] = typeID;

    auto dst = &m_components[index];
    memcpy((void*)dst, (void*)src, sizeof(ComponentInfo));

    dst->index = typeID;

    m_hashMap.emplace(dst->hash, typeID);

    logger::debug("[ ComponentRegistry ] registered: name=%s, hash=0x%llX, index=%d, stride=%ld alignment=%ld",
        dst->name, dst->hash, dst->index, dst->stride, dst->alignment);
}

#include <iostream>
void ComponentRegistry::unregister(TypeID id)
{
    if (id >= m_typeIDToIndex.size())
        return;

    TypeID index = m_typeIDToIndex[id];
    if (index == INVALID_TYPE)
        return;

    ComponentInfo info = m_components[index];

    m_typeIDToIndex[id] = INVALID_TYPE;
    m_hashMap.erase(info.hash);

    TypeID lastIndex = static_cast<TypeID>(--m_count);
    if (index != lastIndex)
    {
        auto dst = m_components + index;
        auto src = m_components + lastIndex;

        TypeID movedTypeID = m_indexToTypeID[lastIndex];

        memcpy(dst, src, sizeof(ComponentInfo));

        m_typeIDToIndex[movedTypeID] = index;
        m_indexToTypeID[index] = movedTypeID;

        std::cout << "Swap" << std::endl;
    }

    logger::debug("[ ComponentRegistry ] unregistered: name=%s, hash=0x%llX, index=%d, stride=%ld alignment=%ld",
        info.name, info.hash, info.index, info.stride, info.alignment);
}

void ComponentRegistry::grow()
{
    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_components = (ComponentInfo*)realloc(m_components, m_capacity * sizeof(ComponentInfo));

    m_indexToTypeID.resize(m_capacity, INVALID_TYPE);
}

const ComponentInfo* ComponentRegistry::getComponentInfoByHash(Hash64 hash) const
{
    auto it = m_hashMap.find(hash);
    if (it == m_hashMap.end())
        return nullptr;
    return getComponentByID(it->second);
}

TypeID ComponentRegistry::getTypeIDByHash(Hash64 hash) const
{
    auto it = m_hashMap.find(hash);
    if (it != m_hashMap.end())
        return it->second;
    return INVALID_TYPE;
}

const ComponentInfo* ComponentRegistry::getComponentByID(TypeID id) const
{
    if (id >= m_typeIDToIndex.size())
        return nullptr;

    TypeID index = m_typeIDToIndex[id];
    if (index == INVALID_TYPE)
        return nullptr;
    return &m_components[index];
}


