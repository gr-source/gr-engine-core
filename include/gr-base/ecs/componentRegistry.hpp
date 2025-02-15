#pragma once

#include <unordered_map>
#include <typeindex>
#include <vector>
#include <string>
#include <memory>

#include "chunkManager.hpp"

class ComponentRegistry
{
public:
    static ComponentRegistry &getInstance();

    static BaseChunk *Find(const std::string &name);

    template <typename T>
    static BaseChunk *Find();

    static void Clear();

    static std::unordered_map<std::string, std::tuple<std::unique_ptr<BaseChunk>, bool>> &GetAllChunkComponent();

    template<typename T>
    bool registry(bool engine = false);
    
    BaseChunk *findScript(const std::string &name) const;
    
    void init();
    
    void update();

    std::vector<std::string> getScriptRequestReinstance() const;

    void removeScriptNeedsUpdate(const std::string &name);

    void CheckScriptRemoved();

private:
    std::unordered_map<std::string, std::tuple<std::unique_ptr<BaseChunk>, bool>> m_scriptChunk;

    std::unordered_map<std::string, std::size_t> m_scriptStride;

    std::vector<std::string> m_scriptToRestart;

    std::unordered_map<std::string, bool> m_scriptRegister;

    std::vector<BaseChunk *> chunkList;

    bool isCanRegister(const std::string &name);

    void removeChunkInvector(BaseChunk *baseChunk);

    void clear();

    void checkScriptStride(const std::string &name, std::size_t stride);

    BaseChunk *registryScript(const std::string &name, std::unique_ptr<BaseChunk> ptr, std::size_t stride, bool isInternal);

    template <typename T>
    static void SetupMethod(BaseChunk *baseChunk);
};

#include "gr-base/core/string.hpp"

template <typename T>
inline BaseChunk *ComponentRegistry::Find()
{
    return Find(String::RemoveNumber(typeid(T).name()));
}

template <typename T>
inline bool ComponentRegistry::registry(bool engine)
{
    auto name = String::RemoveNumber(typeid(T).name());

    checkScriptStride(name, sizeof(T));

    auto baseChunk = Find<T>();
    if (baseChunk != nullptr)
    {
        SetupMethod<T>(baseChunk);
    }
    
    if (!isCanRegister(name))
    {
        return true;
    }
    
    baseChunk = registryScript(name, std::make_unique<ChunkManager<T>>(), sizeof(T), engine);
    
    SetupMethod<T>(baseChunk);
    return true;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
template <typename T>
inline void ComponentRegistry::SetupMethod(BaseChunk *baseChunk)
{
        
    baseChunk->init        = reinterpret_cast<void (*) (Component *)>((void *) &T::Init);
    baseChunk->start       = reinterpret_cast<void (*) (Component *)>((void *) &T::Start);
    baseChunk->update      = reinterpret_cast<void (*) (Component *)>((void *) &T::Update);
    baseChunk->enable      = reinterpret_cast<void (*) (Component *)>((void *) &T::Enable);
    baseChunk->disable     = reinterpret_cast<void (*) (Component *)>((void *) &T::Disable);
    baseChunk->serialize   = reinterpret_cast<bool (*) (Component *, Stream *)>((void *) &T::Serialize);
    baseChunk->deserialize = reinterpret_cast<bool (*) (Component *, Stream *)>((void *) &T::Deserialize);
    baseChunk->destroy     = reinterpret_cast<void (*) (Component *)>((void *) &T::Destroy);

    baseChunk->updateall = reinterpret_cast<void (*)(BaseChunk *)>((void *) &ChunkManager<T>::allupdate);
}
#pragma GCC diagnostic pop
