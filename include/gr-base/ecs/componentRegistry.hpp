#pragma once

#include <unordered_map>
#include <typeindex>
#include <vector>
#include <string>
#include <memory>

#include "gr-base/ecs/componentstorage.hpp"

class ComponentRegistry
{
public:
    template <typename T>
    static bool Register(const std::string &name, bool internal, bool enableupdate);

    static IComponentStorage *Find(const std::string &name);

    static std::vector<std::string> GetScriptsNeedingReinstance();

    static void Update();

    static void Clear();

    // static std::unordered_map<std::string, std::tuple<std::unique_ptr<BaseChunk>, bool>> &GetAllChunkComponent();
 
    // BaseChunk *findScript(const std::string &name) const;
    
    void init();
    
    void update();

    std::vector<std::string> getScriptRequestReinstance() const;

    void removeScriptNeedsUpdate(const std::string &name);

    void CheckScriptRemoved();

private:
    static ComponentRegistry &GetInstance();

    std::unordered_map<std::string, std::unique_ptr<IComponentStorage>> m_componentstorage;

    // std::unordered_map<std::string, std::tuple<std::unique_ptr<BaseChunk>, bool>> m_scriptChunk;

    std::unordered_map<std::string, std::size_t> m_scriptStride;

    std::vector<std::string> m_scriptToRestart;

    std::unordered_map<std::string, bool> m_scriptRegister;

    // std::vector<BaseChunk *> chunkList;

    std::vector<std::string> getScriptsNeedingReinstance();

    template<typename T>
    bool registry(const std::string &name, bool internal, bool enableupdate);

    IComponentStorage *find(const std::string &name);

    bool isCanRegister(const std::string &name);

    void removeChunkInvector(BaseChunk *baseChunk);

    void clear();

    void checkScriptStride(const std::string &name, std::size_t stride);

    // BaseChunk *registryScript(const std::string &name, std::unique_ptr<BaseChunk> ptr, std::size_t stride, bool isInternal);

  
    // template <typename T>
    // static void SetupMethod(BaseChunk *baseChunk, bool bEnableUpdate);
};

// public
template <typename T>
bool ComponentRegistry::Register(const std::string &name, bool internal, bool enableupdate)
{
    return GetInstance().registry<T>(name, internal, enableupdate);
}

#include "gr-base/core/string.hpp"

template <typename T>
inline bool ComponentRegistry::registry(const std::string &name, bool uinternal, bool enableUpdate)
{
    {
        auto it = m_componentstorage.find(name);
        if (it != m_componentstorage.end())
        {
            return false;
        }
    }
    auto &&[it, added] = m_componentstorage.emplace(name, std::make_unique<ComponentStorage<T>>());
    if (added)
    {
        auto componentstorage = static_cast<ComponentStorage<T> *>(it->second.get());

        if constexpr (has_Init_method<T>::value)
            componentstorage->register_method("Init", &T::Init);

        if constexpr (has_Start_method<T>::value)
            componentstorage->register_method("Start", &T::Start);

        if constexpr (has_Enable_method<T>::value)
            componentstorage->register_method("Enable", &T::Enable);

        if constexpr (has_Disable_method<T>::value)
            componentstorage->register_method("Disable", &T::Disable);

        if constexpr (has_Serialize_method<T>::value)
            componentstorage->register_method("Serialize", &T::Serialize);

        if constexpr (has_Deserialize_method<T>::value)
            componentstorage->register_method("Deserialize", &T::Deserialize);

        if constexpr (has_Destroy_method<T>::value)
            componentstorage->register_method("Destroy", &T::Destroy);
    }
    /*
    auto name = String::RemoveNumber(typeid(T).name());

    checkScriptStride(name, sizeof(T));

    auto baseChunk = Find<T>();
    if (baseChunk != nullptr)
    {
        SetupMethod<T>(baseChunk, bEnableUpdate);
    }
    
    if (!isCanRegister(name))
    {
        return true;
    }
    
    baseChunk = registryScript(name, std::make_unique<ChunkManager<T, CHUNK_SIZE>>(), sizeof(T), bInternal);
    
    SetupMethod<T>(baseChunk, bEnableUpdate);
    */
    return true;
}

/*
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpmf-conversions"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#endif

template <typename T>
inline void ComponentRegistry::SetupMethod(BaseChunk *baseChunk, bool bEnableUpdate)
{
    baseChunk->bCanEnableUpdate = bEnableUpdate;

    baseChunk->init        = reinterpret_cast<void (*) (Component *)>((void *) &T::Init);
    baseChunk->start       = reinterpret_cast<void (*) (Component *)>((void *) &T::Start);
    baseChunk->update      = reinterpret_cast<void (*) (Component *)>((void *) &T::Update);
    baseChunk->enable      = reinterpret_cast<void (*) (Component *)>((void *) &T::Enable);
    baseChunk->disable     = reinterpret_cast<void (*) (Component *)>((void *) &T::Disable);
    baseChunk->serialize   = reinterpret_cast<bool (*) (Component *, Stream *)>((void *) &T::Serialize);
    baseChunk->deserialize = reinterpret_cast<bool (*) (Component *, Stream *)>((void *) &T::Deserialize);
    baseChunk->destroy     = reinterpret_cast<void (*) (Component *)>((void *) &T::Destroy);

    baseChunk->initall = reinterpret_cast<void (*)(BaseChunk *)>((void *) &ChunkManager<T, CHUNK_SIZE>::allinit);
    baseChunk->updateall = reinterpret_cast<void (*)(BaseChunk *)>((void *) &ChunkManager<T, CHUNK_SIZE>::allupdate);

    baseChunk->allocatein = reinterpret_cast<Component *(*)(BaseChunk *, Entity *, const Component *)>((void *) &ChunkManager<T, CHUNK_SIZE>::allocateComponent);
    baseChunk->deallocatein = reinterpret_cast<void (*)(BaseChunk *, Component *)>((void *) &ChunkManager<T, CHUNK_SIZE>::deallocate);
}





#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif
*/


