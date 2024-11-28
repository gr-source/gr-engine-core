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

void ComponentRegistry::Clean() {
    auto &instance = getInstance();

    for (auto [name, pool] : instance.m_poolMap) {
        instance.m_sizeKlass[name] = pool->getSize();
    }
    
    instance.m_poolMap.clear();

    instance.m_pool.clear();
}

std::vector<std::string> ComponentRegistry::GetRequestScriptUpdate() {
    auto &instance = getInstance();

    auto temp = instance.m_script;
    instance.m_script.clear();

    return temp;
}
