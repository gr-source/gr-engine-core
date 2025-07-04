#include "gr-base/ecs/icomponentstorage.hpp"

void IComponentStorage::init(Component *component)
{
    if (hasMethod("Init_0"))
        return m_methodmap["Init_0"]->invoke<void>((void *)component);
}

void IComponentStorage::start(Component *component)
{
    if (hasMethod("Start_0"))
        return m_methodmap["Start_0"]->invoke<void>((void *)component);
}

void IComponentStorage::enable(Component *component)
{
    if (hasMethod("Enable_0"))
        return m_methodmap["Enable_0"]->invoke<void>((void *)component);
}

void IComponentStorage::disable(Component *component)
{
    if (hasMethod("Disable_0"))
        return m_methodmap["Disable_0"]->invoke<void>((void *)component);
}

bool IComponentStorage::serialize(Component *component, Stream *stream)
{
    if (hasMethod("Serialize_1"))
        return m_methodmap["Serialize_1"]->invoke<bool>((void *)component, stream);
    return true;
}

bool IComponentStorage::deserialize(Component *component, Stream *stream)
{
    if (hasMethod("Deserialize_1"))
        return m_methodmap["Deserialize_1"]->invoke<bool>((void *)component, stream);
    return true;
}

void IComponentStorage::destroy(Component *component)
{
    if (hasMethod("Destroy_0"))
        return m_methodmap["Destroy_0"]->invoke<void>((void *)component);
}

bool IComponentStorage::hasMethod(const std::string &name) const
{
    return m_methodmap.find(name) != m_methodmap.end();
}

