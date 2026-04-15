#pragma once

#include "gr-base/externals/utils.h"

#include <string>
#include <unordered_map>

typedef uint64_t AssetHash;

class AssetReference
{
public:
    static void Initialize();

    static void Shutdown();

    static void Load(const std::string &file);

    static void Save(const std::string &file);

    static void Registry(const std::string &name, const std::string& file);

    static std::string Get(AssetHash hash);

private:
    static AssetReference* s_AssetReference;

    std::unordered_map<std::string, std::string> m_refMapped;

    std::unordered_map<AssetHash, std::string> m_refHashMapped;

    void load(const std::string &file);

    void save(const std::string &file);

    void registry(const std::string &name, const std::string& file);

    std::string get(AssetHash hash);
};


