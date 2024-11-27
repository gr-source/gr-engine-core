#pragma once

#include "componentPool.hpp"

#include <memory>

class __attribute__((visibility("default"))) ComponentRegistry {
public:
    static ComponentRegistry &getInstance();

    static Pool *Find(const std::string &name);

    static std::unordered_map<std::string, Pool *> GetPool();

    static void Clean();

    template <typename T>
    bool RegisterComponent();

    template <typename T>
    ComponentPool<T> *GetComponentPool();

private:
    std::unordered_map<std::string, Pool *> m_poolMap;

    std::vector<std::unique_ptr<Pool>> m_pool;
};

static std::string nRemoveNumber(const std::string &text) {
    std:: string result;
    for (auto & ch : text) {
        if (std::isdigit(ch)) {
            continue;
        }
        result += ch;
    }
    return result;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
template <typename T>
inline bool ComponentRegistry::RegisterComponent() {
    auto name = nRemoveNumber(typeid(T).name());

    auto it = m_poolMap.find(name);
    if (it != m_poolMap.end()) {
        return true;
    }
    auto &ptr = m_pool.emplace_back(std::make_unique<ComponentPool<T>>());

    auto &&[iterator, added] = m_poolMap.emplace(name, &(*ptr));
    if (!added) {
        throw std::runtime_error("Component: " + it->first + " replaced.");
    }
    ptr->init = reinterpret_cast<void (*) (Component *)>((void *) &T::Init);
    ptr->start = reinterpret_cast<void (*) (Component *)>((void *) &T::Start);
    ptr->update = reinterpret_cast<void (*) (Component *)>((void *) &T::Update);
    ptr->enable = reinterpret_cast<void (*) (Component *)>((void *) &T::Enable);
    ptr->disable = reinterpret_cast<void (*) (Component *)>((void *) &T::Disable);
    ptr->serialize = reinterpret_cast<bool (*) (Component *, Stream *)>((void *) &T::Serialize);
    ptr->deserialize = reinterpret_cast<bool (*) (Component *, Stream *)>((void *) &T::Deserialize);
    ptr->destroy = reinterpret_cast<void (*) (Component *)>((void *) &T::Destroy);
    
    return true;
}

template <typename T>
inline ComponentPool<T> *ComponentRegistry::GetComponentPool() {
    auto it = m_poolMap.find(nRemoveNumber(typeid(T).name()));
    if (it == m_poolMap.end()) {
        throw std::runtime_error("Component do not added: " + std::string(nRemoveNumber(typeid(T).name())));
    }
    auto ptr = &(*it->second);
    if (!ptr) {
        throw std::runtime_error("Ivalid component pointer: " + std::string(nRemoveNumber(typeid(T).name())));
    }

    auto typedPool = static_cast<ComponentPool<T> *>(ptr);
    if (!typedPool) {
        throw std::runtime_error("Error in type cast.");
    }
    return typedPool;
}
#pragma GCC diagnostic pop

