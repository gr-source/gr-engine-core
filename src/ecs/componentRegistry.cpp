#include "ecs/componentRegistry.hpp"

ComponentRegistry &ComponentRegistry::getInstance() {
    static ComponentRegistry m_instance;
    return m_instance;
}

Pool *ComponentRegistry::Find(const std::string &name) {
    auto it = getInstance().m_poolMap.find(name);
    if (it == getInstance().m_poolMap.end()) {
        return nullptr;
    }
    return &(*it->second);
}

std::unordered_map<std::string, Pool *> ComponentRegistry::GetPoolList() {
    return getInstance().m_poolMap;
}

std::vector<std::string> ComponentRegistry::GetRequestScriptUpdate() {
    auto &instance = getInstance();

    auto temp = instance.m_script;
    instance.m_script.clear();

    return temp;
}

void ComponentRegistry::clear() {
    for (auto [name, pool] : m_poolMap) {
        m_sizeKlass[name] = pool->getSize();
    }
    
    m_poolMap.clear();

    m_pool.clear();
}











