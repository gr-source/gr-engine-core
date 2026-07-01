#pragma once

#include "gr-base/asset/default.hpp"

#include "gr-base/pool/sparseSet.hpp"

#include <unordered_map>
#include <string>

#define ASSET_MAX_PATH_LENGHT 1024

typedef struct AssetInfo
{
    AssetHash hash;
    AssetID id;

    char path[ASSET_MAX_PATH_LENGHT];
} AssetInfo;

class AssetDB
{
public:
    static void Initialize();

    static void Shutdown();

    static void Load(const std::string &file);

    static void Save(const std::string &file);

    static void Registry(AssetHash hash, const std::string& path);

    static void Unregistry(AssetHash hash);

    static std::string Resolve(Hash64 hash);

private:
    static AssetDB* s_AssetDB;

    std::unordered_map<Hash64, AssetID> m_refMapped;

    SparseSet<AssetInfo> m_spaseSet;

    AssetID m_nextID;

    AssetDB();
    ~AssetDB();

    void load(const std::string &file);

    void save(const std::string &file);

    void registry(Hash64 hash, const std::string& path);

    void unregistry(Hash64 hash);

    void insert(const AssetInfo& info);

    std::string resolve(Hash64 hash);
};


