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
    std::cout << "Clean: " << instance.m_pool.size() << std::endl;
    
    instance.m_poolMap.clear();
    instance.m_pool.clear();

}






