#include "gr-base/ecs/systemRegistry.hpp"

#include "gr-base/system/default.hpp"

#include "gr-base/core/logger.hpp"

SystemRegistry* SystemRegistry::s_SystemRegistry = nullptr;

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

// private
SystemRegistry::SystemRegistry()
{
    logger::info("[ SystemRegistry ] Initialize '%p'.", this);
}

SystemRegistry::~SystemRegistry()
{
    logger::info("[ SystemRegistry ] Shutdown '%p'.", this);
}

void SystemRegistry::register_info(SystemInfo* src)
{
    if (m_registeredTypeIDs.find(src->hash) != m_registeredTypeIDs.end())
        return logger::warn("[ SystemRegistry ] type already registered '%s'.", src->name);

    if (m_persistentTypeIDs.find(src->hash) == m_persistentTypeIDs.end())
        m_persistentTypeIDs.emplace(src->hash, static_cast<TypeID>(m_nextID++));

    TypeID typeID = m_persistentTypeIDs[src->hash];

    src->index = typeID;
    sparseSet.insert(typeID, *src);

    m_registeredTypeIDs.emplace(src->hash, typeID);

    logger::debug("[ SystemRegistry ] registered: name=%s, hash=0x%llX, index=%d, stride=%ld alignment=%ld",
        src->name, src->hash, src->index, src->stride, src->alignment);
}

void SystemRegistry::unregister(TypeID id)
{
    auto info = sparseSet.try_get(id);
    if (info == nullptr)
        return;

    logger::debug("[ SystemRegistry ] unregistered: name=%s, hash=0x%llX, index=%d, stride=%ld alignment=%ld",
        info->name, info->hash, info->index, info->stride, info->alignment);

    m_registeredTypeIDs.erase(info->hash);

    sparseSet.destroy(id);
}

const SystemInfo* SystemRegistry::getSystemByHash(Hash64 hash) const
{
    auto it = m_registeredTypeIDs.find(hash);
    if (it == m_registeredTypeIDs.end())
        return nullptr;
    return getSystemByID(it->second);
}

const SystemInfo* SystemRegistry::getSystemByID(TypeID id) const
{
    return sparseSet.try_get(id);
}

TypeID SystemRegistry::getTypeIDByHash(Hash64 hash) const
{
    auto it = m_registeredTypeIDs.find(hash);
    if (it != m_registeredTypeIDs.end())
        return it->second;
    return INVALID_TYPE;
}


