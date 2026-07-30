#include "gr-base/asset/assetDB.hpp"

#include "gr-base/asset/URI.hpp"
#include "gr-base/asset/assetResolver.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/core/string.hpp"
#include "gr-base/externals/hash.hpp"
#include "gr-base/managers/eventManager.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

AssetDB* AssetDB::s_AssetDB = nullptr;

void AssetDB::Initialize()
{
    if (s_AssetDB == nullptr)
        s_AssetDB = new AssetDB();
}

void AssetDB::Shutdown()
{
    if (s_AssetDB != nullptr)
        delete s_AssetDB;

    s_AssetDB = nullptr;
}

void AssetDB::Load(const std::string &file)
{
    s_AssetDB->load(file);
}

void AssetDB::Save(const std::string &file)
{
    s_AssetDB->save(file);
}

void AssetDB::Registry(const AssetInfo& info)
{
    s_AssetDB->registry(info);
}
/*
void AssetDB::Registry(const std::string& path, AssetType flags)
{
    s_AssetDB->registry(path, flags);
}

void AssetDB::Unregistry(Hash64 hash)
{
    s_AssetDB->unregistry(hash);
}

std::string AssetDB::Resolve(Hash64 hash)
{
    return s_AssetDB->resolve(hash);
}
*/

AssetID AssetDB::FindByHash(AssetHash hash)
{
    return s_AssetDB->findByHash(hash);
}

AssetID AssetDB::CreateID()
{
    return s_AssetDB->createID();
}

AssetInfo* AssetDB::GetAsset(AssetID id)
{
    return s_AssetDB->m_spaseSet.get(id);
}

// private
AssetDB::AssetDB() : m_nextID(0)
{
    logger::info("[ AssetDB ] Initialize.");
}

AssetDB::~AssetDB()
{
    logger::info("[ AssetDB ] Shutdown.");
}

void AssetDB::load(const std::string &file)
{
    std::ifstream is(file);
    if (!is.is_open())
        return logger::error("[AssetDB] Fail in to load source '%s'.", strerror(errno));

    logger::info("[AssetDB] loading ref: '%s'.", file.c_str());

    std::string line;
    while (std::getline(is, line))
    {
        std::string path;
        AssetID id;
        Hash64 hash;
        int type;

        std::stringstream iss(line);
        if (!(iss >> hash >> id >> path >> type))
            continue;

        AssetInfo info;
        info.hash = hash;
        info.id = id;
        info.type = static_cast<AssetType>(type);

        size_t size = path.size();
        size = size > ASSET_MAX_PATH_LENGHT ? ASSET_MAX_PATH_LENGHT : size;

        strncpy(info.uri, path.c_str(), size);
        info.uri[size] = '\0';

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (id >= m_nextID)
                m_nextID = id + 1;
        }

        registry(info);
    }

    EventManager::Notify<AssetEventOnLoaded>(nullptr);

    is.close();
}

void AssetDB::save(const std::string &file)
{
    std::ofstream os(file);

    const auto infos = m_spaseSet.data();
    for (size_t i=0;i<m_spaseSet.count();i++)
    {
        AssetInfo& info = infos[i];

        os
            << info.hash  << " "
            << info.id    << " "
            << info.uri  << " "
            << static_cast<int>(info.type)
            << std::endl;
    }

    os.close();
}

/*
void AssetDB::registry(const std::string& path, AssetType type)
{
    AssetHash hash = engine::hash::fnv1_str(path);

    if (m_refMapped.find(hash) != m_refMapped.end())
        return;

    AssetID id = createID();

    AssetInfo info;
    info.hash = hash;
    info.id = id;
    info.type = type;

    size_t size = path.size();
    size = size > ASSET_MAX_PATH_LENGHT ? ASSET_MAX_PATH_LENGHT : size;

    strncpy(info.uri, path.c_str(), size);
    info.uri[size] = '\0';

    insert(info);
}

void AssetDB::unregistry(Hash64 hash)
{
    auto it = m_refMapped.find(hash);
    if (it == m_refMapped.end())
        return;

    AssetEventOnRemove event;
    event.value = it->second;

    EventManager::Notify<AssetEventOnRemove>(&event);

    auto info = m_spaseSet.get(it->second);

    logger::trace("[ AssetDB ] unregister: '0x%lX' lookup: '%s'.", hash, info->uri);

    m_spaseSet.destroy(it->second);

    m_refMapped.erase(it);
}
*/

void AssetDB::registry(const AssetInfo& info)
{
    URI uri = URI::parse(info.uri);

    m_spaseSet.insert(info.id, info);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_refMapped.emplace(info.hash, info.id);
    }

    AssetEventOnCreated event;
    event.value = info.id;

    EventManager::Notify<AssetEventOnCreated>(&event);
}

/*
std::string AssetDB::resolve(Hash64 hash)
{
    AssetID id = findByHash(hash);
    if (id == INVALID_ASSET)
        throw std::runtime_error(String::format("reference '0x%lX' not found...", hash));

    auto info = m_spaseSet.get(id);
    if (info == nullptr)
        throw std::runtime_error(String::format("this assetID is not valid id='%d'.", id));

    return AssetResolver::Resolve(URI::parse(info->uri));
}
*/

AssetID AssetDB::findByHash(AssetHash hash)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_refMapped.find(hash);
    if (it != m_refMapped.end())
        return it->second;
    return INVALID_ASSET;
}

AssetID AssetDB::createID()
{
    AssetID id;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        id = m_nextID++;
    }
    return id;
}


