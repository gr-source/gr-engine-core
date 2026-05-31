#pragma once

#include "gr-base/externals/hash.hpp"
#include "gr-base/externals/utils.h"

#include "gr-base/core/default.hpp"

#include <unordered_map>
#include <string>

struct AssetConfig
{
    std::string projectRoot;
    std::string assetFolder;
};

class AssetDB
{
public:
    static void Initialize(const AssetConfig& config);

    static void Shutdown();

    static void Load(const std::string &file);

    static void Save(const std::string &file);

    static void Registry(AssetHash hash, const std::string& path);

    static void Unregistry(AssetHash hash);

    static std::string Resolve(Hash64 hash);

private:
    static AssetDB* s_AssetDB;

    std::unordered_map<Hash64, std::string> m_refMapped;

    AssetConfig m_config;

    AssetDB(const AssetConfig& config);
    ~AssetDB();

    void load(const std::string &file);

    void save(const std::string &file);

    void registry(Hash64 hash, const std::string& path);

    void unregistry(Hash64 hash);

    std::string resolve(Hash64 hash);
};


