#include "gr-base/ecs/componentRegistry.hpp"

#include "gr-base/core/logger.hpp"

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


TypeID ComponentRegistry::GetTypeIDByName(const char* name)
{
    return s_ComponentRegistry->getTypeIDByHash(engine::hash::fnv1_str(name));
}

const ComponentInfo* ComponentRegistry::GetComponentByID(TypeID id)
{
    return s_ComponentRegistry->getComponentByID(id);
}

// private
ComponentRegistry::ComponentRegistry()
{
    logger::info("[ ComponentRegistry ] Initialize '%p'.", this);
}

ComponentRegistry::~ComponentRegistry()
{
    logger::info("[ ComponentRegistry ] Shutdown '%p'.", this);
}

void ComponentRegistry::register_info(ComponentInfo* src)
{
    if (m_registeredTypeIDs.find(src->hash) != m_registeredTypeIDs.end())
        return logger::warn("[ ComponentRegistry ] type already registered '%s'.", src->name);

    if (m_persistentTypeIDs.find(src->hash) == m_persistentTypeIDs.end())
        m_persistentTypeIDs.emplace(src->hash, static_cast<TypeID>(m_nextID++));

    TypeID typeID = m_persistentTypeIDs[src->hash];

    src->index = typeID;
    sparseSet.insert(typeID, *src);

    m_registeredTypeIDs.emplace(src->hash, typeID);

    logger::debug("[ ComponentRegistry ] registered: name=%s, hash=0x%llX, index=%d, stride=%ld alignment=%ld",
        src->name, src->hash, src->index, src->stride, src->alignment);
}

void ComponentRegistry::unregister(TypeID id)
{
    auto info = sparseSet.try_get(id);
    if (info == nullptr)
        return;

    logger::debug("[ ComponentRegistry ] unregistered: name=%s, hash=0x%llX, index=%d, stride=%ld alignment=%ld",
        info->name, info->hash, info->index, info->stride, info->alignment);

    m_registeredTypeIDs.erase(info->hash);

    sparseSet.destroy(id);
}

const ComponentInfo* ComponentRegistry::getComponentInfoByHash(Hash64 hash) const
{
    auto it = m_registeredTypeIDs.find(hash);
    if (it == m_registeredTypeIDs.end())
        return nullptr;
    return getComponentByID(it->second);
}

TypeID ComponentRegistry::getTypeIDByHash(Hash64 hash) const
{
    auto it = m_registeredTypeIDs.find(hash);
    if (it != m_registeredTypeIDs.end())
        return it->second;
    return INVALID_TYPE;
}

const ComponentInfo* ComponentRegistry::getComponentByID(TypeID id) const
{
    return sparseSet.try_get(id);
}


