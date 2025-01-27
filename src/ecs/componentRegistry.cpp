#include "gr-base/ecs/componentRegistry.hpp"

ComponentRegistry &ComponentRegistry::getInstance() {
    static ComponentRegistry m_instance;
    return m_instance;
}

Pool *ComponentRegistry::Find(const std::string &name) {
    auto it = getInstance().m_poolMap.find(name);
    if (it == getInstance().m_poolMap.end()) {
        return nullptr;
    }
    return it->second.pointer;
}

std::unordered_map<std::string, PoolData> ComponentRegistry::GetPoolList() {
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
        m_sizeKlass[name] = pool.pointer->getSize();
    }

    for (auto it = m_poolMap.begin(); it != m_poolMap.end();) {
        if (!it->second.system) {
            delete it->second.pointer;

            it = m_poolMap.erase(it);
            
            continue;
        }
        it++;
    } 
    
    // m_poolMap.clear();

    // m_pool.clear();
}

ComponentRegistry::ComponentRegistry() { }

ComponentRegistry::~ComponentRegistry() {
    for (auto [name, data] : m_poolMap) {
        delete data.pointer;
    }
}
