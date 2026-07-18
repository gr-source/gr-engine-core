#pragma once

#include "gr-base/asset/URI.hpp"
#include "gr-base/asset/default.hpp"

#include "gr-base/externals/hash.hpp"
#include "gr-base/pool/sparseSet.hpp"

#include <unordered_map>
#include <vector>
#include <string>

#define ASSET_MAX_PATH_LENGHT 1024

typedef struct AssetInfo
{
    AssetHash hash = INVALID_HASH64;
    AssetID id = INVALID_ASSET;
    AssetFlags_ flags = 0;

    char path[ASSET_MAX_PATH_LENGHT] = {0};
} AssetInfo;

class AssetDB
{
public:
    static void Initialize();

    static void Shutdown();

    static void Load(const std::string &file);

    static void Save(const std::string &file);

    static void Registry(const std::string& path, AssetFlags_ flags);

    static void Unregistry(AssetHash hash);

    static std::string Resolve(Hash64 hash);

    static std::vector<AssetID> GetAssets(Scheme scheme);

    static AssetInfo* GetAsset(AssetID id);

private:
    static AssetDB* s_AssetDB;

    std::unordered_map<Hash64, AssetID> m_refMapped;

    SparseSet<AssetInfo> m_spaseSet;

    std::vector<AssetID> m_assets[static_cast<size_t>(Scheme::Count)];

    AssetID m_nextID;

    AssetDB();
    ~AssetDB();

    void load(const std::string &file);

    void save(const std::string &file);

    void registry(const std::string& path, AssetFlags_ flags);

    void unregistry(Hash64 hash);

    void insert(const AssetInfo& info);

    std::string resolve(Hash64 hash);
};


