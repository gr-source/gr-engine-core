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
    m_systems(nullptr)
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

    TypeID id = static_cast<TypeID>(m_count++);

    SystemInfo* dest = m_systems + id;
    memcpy((void*)dest, (void*)src, sizeof(SystemInfo));

    dest->index = id;

    m_hashMap.emplace(dest->hash, id);

    logger::debug("[ SystemRegistry ] registered: name=%s, hash=%llu, index=%d, stride=%ld alignment=%ld", \
        dest->name, dest->hash, dest->index, dest->stride, dest->alignment); \
}

void SystemRegistry::grow()
{
    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_systems = (SystemInfo*)realloc(m_systems, m_capacity * sizeof(SystemInfo));
}

const SystemInfo* SystemRegistry::getSystemInfoByHash(Hash64 hash) const
{
    auto it = m_hashMap.find(hash);
    if (it != m_hashMap.end())
        return m_systems + it->second;
    return nullptr;
}

TypeID SystemRegistry::getTypeIDByHash(Hash64 hash) const
{
    auto it = m_hashMap.find(hash);
    if (it != m_hashMap.end())
        return it->second;
    return INVALID_TYPE;
}

const SystemInfo* SystemRegistry::getSystemInfoByID(TypeID id)
{
    assert(id < componentCount);

    return m_systems + id;
}


