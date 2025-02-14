#pragma once

#include <unordered_map>
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
    
    BaseChunk *findScript(const char *name) const;
    
    void init();
    
    void update();

private:
    std::unordered_map<std::string, std::tuple<std::unique_ptr<BaseChunk>, bool>> m_scriptChunk;

    std::vector<BaseChunk *> chunkList;

    void clear();
    
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

    auto it = m_scriptChunk.find(name.c_str());
    if (it != m_scriptChunk.end())
    {
        return true;
    }
    
    m_scriptChunk.emplace(name.c_str(), std::make_tuple(std::make_unique<ChunkManager<T>>(), engine));

    auto baseChunk = &(*std::get<0>(m_scriptChunk[name.c_str()]));
    

    chunkList.push_back(baseChunk);
    
    return true;
}
