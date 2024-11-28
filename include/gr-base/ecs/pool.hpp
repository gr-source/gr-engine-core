#pragma once

#include "field.hpp"

#include <functional>

enum class FuncType {
    UPDATE,
    INIT,
    DESTROY,
};

class Stream;

class Pool {
public:
    static Component *CreatComponent(std::size_t size, Entity *entity);

    Pool();

    virtual ~Pool() {}

    virtual Component *CreateInstance(const Component *component, Entity *entity) = 0;

    virtual Component *Instance(Component *component) = 0;

    template <typename ...Args>
    bool RegisterField(const std::string &name, Args &&...args);

    std::vector<Field> getFields() const;

    void (*init)(Component *component);

    void (*start)(Component *component);

    void (*update)(Component *component);

    void (*enable)(Component *component);

    void (*disable)(Component *component);

    bool (*serialize)(Component *component, Stream *streaming);

    bool (*deserialize)(Component *component, Stream *streaming);

    void (*destroy)(Component *component);
    
private:
    std::unordered_map<std::string, grm::size> m_fieldID;

    std::vector<Field> m_fieldMap;
};

template <typename... Args>
inline bool Pool::RegisterField(const std::string &name, Args &&...args) {
    auto it = m_fieldID.find(name);
    if (it != m_fieldID.end()) {
        return true;
    }

    auto id = m_fieldMap.size();
    m_fieldMap.emplace_back(name, std::forward<Args>(args)...);
    m_fieldID.emplace(name, id);
    return true;
}



