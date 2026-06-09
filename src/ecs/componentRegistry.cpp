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
    m_components(nullptr)
{
    logger::info("[ ComponentRegistry ] Initialize '%p'.", this);
}

ComponentRegistry::~ComponentRegistry()
{
    logger::info("[ ComponentRegistry ] Shutdown '%p'.", this);
}

void ComponentRegistry::register_info(ComponentInfo* src)
{
    if (m_hashMap.find(src->hash) != m_hashMap.end())
        return logger::warn("[ ComponentRegistry ] type already registered '%s'.", src->name);

    if (m_count >= m_capacity)
        grow();

    TypeID id = static_cast<TypeID>(m_count++);

    ComponentInfo* dest = m_components + id;
    memcpy((void*)dest, (void*)src, sizeof(ComponentInfo));

    dest->index = id;

    m_hashMap.emplace(dest->hash, id);

    logger::debug("[ ComponentRegistry ] registered: name=%s, hash=%llu, index=%d, stride=%ld alignment=%ld", \
        dest->name, dest->hash, dest->index, dest->stride, dest->alignment); \
}

void ComponentRegistry::grow()
{
    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_components = (ComponentInfo*)realloc(m_components, m_capacity * sizeof(ComponentInfo));
}

const ComponentInfo* ComponentRegistry::getComponentInfoByHash(Hash64 hash) const
{
    auto it = m_hashMap.find(hash);
    if (it != m_hashMap.end())
        return m_components + it->second;
    return nullptr;
}

TypeID ComponentRegistry::getTypeIDByHash(Hash64 hash) const
{
    auto it = m_hashMap.find(hash);
    if (it != m_hashMap.end())
        return it->second;
    return INVALID_TYPE;
}

const ComponentInfo* ComponentRegistry::getComponentInfoByID(TypeID id)
{
    assert(id < componentCount);

    return m_components + id;
}


