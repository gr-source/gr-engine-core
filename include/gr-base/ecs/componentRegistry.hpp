#pragma once

#include <unordered_set>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <string>
#include <memory>

#include "chunkManager.hpp"

struct ComponentInfo
{
    std::string name;

    std::size_t stride;

    bool is_engine;

    IComponentStorage *storage;
};

class ComponentRegistry
{
public:
    template <typename T>
    static bool Register(const std::string &name, bool is_engine);

    static IComponentStorage *Find(const std::string &name);

    static std::vector<std::string> GetScriptRequestReinstance();

    static void Update();

    static void CheckScriptRemoved();

    static void Clear();

    static std::vector<ComponentInfo *> &GetComponentInfoList();
 
    IComponentStorage *find(const std::string &name) const;
    
    void init();
    
    void update();

    void removeScriptNeedsUpdate(const std::string &name);

    void check_script_removed();

private:
    static ComponentRegistry &GetInstance();

    std::unordered_map<std::string, std::unique_ptr<IComponentStorage>> m_componentstoragemap;

    std::unordered_map<std::string, std::unique_ptr<ComponentInfo>> m_componentinfomap;

    std::vector<std::string> m_scriptToRestart;

    std::unordered_set<std::string> m_scriptRegister;

    std::vector<ComponentInfo *> m_componentinfolist;

    template<typename T>
    bool registry(const std::string &name, bool is_engine);

    bool isCanRegister(const std::string &name);

    void remove_component(const std::string &name);

    void clear();

    void checkScriptStride(const std::string &name, std::size_t stride);

    IComponentStorage *registryScript(const std::string &name, std::unique_ptr<IComponentStorage> ptr, std::size_t stride, bool isInternal);

    template <typename T>
    static void SetupMethod(IComponentStorage *storage);
};

// public
template <typename T>
bool ComponentRegistry::Register(const std::string &name, bool is_engine)
{
    return GetInstance().registry<T>(name, is_engine);
}

#include "gr-base/core/string.hpp"

// private
template <typename T>
inline bool ComponentRegistry::registry(const std::string &name, bool is_engine)
{
    checkScriptStride(name, sizeof(T));

    auto storage = find(name);
    if (storage != nullptr)
    {
        SetupMethod<T>(storage);
    }
    
    if (!isCanRegister(name))
    {
        return true;
    }
    
    storage = registryScript(name, std::make_unique<ComponentStorage<T>>(), sizeof(T), is_engine);
    
    SetupMethod<T>(storage);
    return true;
}

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpmf-conversions"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#endif

template <typename T>
inline void ComponentRegistry::SetupMethod(IComponentStorage *storage)
{
    storage->bCanEnableUpdate = has_Update_method<T>::value;

    storage->init        = reinterpret_cast<void (*) (Component *)>((void *) &T::Init);
    storage->start       = reinterpret_cast<void (*) (Component *)>((void *) &T::Start);
    storage->update      = reinterpret_cast<void (*) (Component *)>((void *) &T::Update);
    storage->enable      = reinterpret_cast<void (*) (Component *)>((void *) &T::Enable);
    storage->disable     = reinterpret_cast<void (*) (Component *)>((void *) &T::Disable);
    storage->serialize   = reinterpret_cast<bool (*) (Component *, Stream *)>((void *) &T::Serialize);
    storage->deserialize = reinterpret_cast<bool (*) (Component *, Stream *)>((void *) &T::Deserialize);
    storage->destroy     = reinterpret_cast<void (*) (Component *)>((void *) &T::Destroy);

    storage->initall = reinterpret_cast<void (*)(IComponentStorage *)>((void *) &ComponentStorage<T>::allinit);
    storage->updateall = reinterpret_cast<void (*)(IComponentStorage *)>((void *) &ComponentStorage<T>::allupdate);

    storage->allocatein = reinterpret_cast<Component *(*)(IComponentStorage *, Entity *, const Component *)>((void *) &ComponentStorage<T>::allocateComponent);
    storage->deallocatein = reinterpret_cast<void (*)(IComponentStorage *, Component *)>((void *) &ComponentStorage<T>::deallocate);
}





#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif



