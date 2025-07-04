#pragma once

#include "default.hpp"

#include "imethodinvoke.hpp"

#include "gr-base/core/file/default.hpp"

class IComponentStorage
{
public:
    virtual ~IComponentStorage() {}

    virtual Component *allocate(Entity *entity, const Component *other = nullptr) = 0;

    virtual void deallocate(Component *component) = 0;

    template <typename T, typename ...Args>
    T *Create(Args &&...args)
    {
        return nullptr;
    }

    void init(Component *component);

    void start(Component *component);
    
    void enable(Component *component);

    void disable(Component *component);

    bool serialize(Component *component, Stream *stream);

    bool deserialize(Component *component, Stream *stream);

    void destroy(Component *component);

    bool hasMethod(const std::string &name) const;

    std::unordered_map<std::string, std::unique_ptr<IMethodInvoke>> m_methodmap;
};

