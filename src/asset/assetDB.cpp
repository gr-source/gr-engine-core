#include "gr-base/asset/assetDB.hpp"

#include "gr-base/asset/assetResolver.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/core/string.hpp"

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

void AssetDB::Registry(Hash64 hash, const std::string& path)
{
    s_AssetDB->registry(hash, path);
}

void AssetDB::Unregistry(Hash64 hash)
{
    s_AssetDB->unregistry(hash);
}

std::string AssetDB::Resolve(Hash64 hash)
{
    return s_AssetDB->resolve(hash);
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

        std::stringstream iss(line);
        if (!(iss >> hash >> id >> path))
            continue;

        AssetInfo info;
        info.hash = hash;
        info.id = id;

        size_t size = path.size();
        size = size > ASSET_MAX_PATH_LENGHT ? ASSET_MAX_PATH_LENGHT : size;

        strncpy(info.path, path.c_str(), size);
        info.path[size] = '\0';

        if (id >= m_nextID)
            m_nextID = id + 1;

        insert(info);
    }

    is.close();
}

void AssetDB::save(const std::string &file)
{
    std::ofstream os(file);

    const auto infos = m_spaseSet.data();
    for (size_t i=0;i<m_spaseSet.count();i++)
    {
        AssetInfo& info = infos[i];

        os << info.hash << " " << info.id << " " << info.path << std::endl;
    }
    /*
    for (auto &&[hash, info] : m_refMapped)
    {
        os << info.hash << " " << info.id << " " << info.path << std::endl;
    }
    */
    os.close();
}

void AssetDB::registry(Hash64 hash, const std::string& path)
{
    if (m_refMapped.find(hash) != m_refMapped.end())
        return;

    AssetID id = static_cast<AssetID>(m_nextID++);

    AssetInfo info;
    info.hash = hash;
    info.id = id;

    size_t size = path.size();
    size = size > ASSET_MAX_PATH_LENGHT ? ASSET_MAX_PATH_LENGHT : size;

    strncpy(info.path, path.c_str(), size);
    info.path[size] = '\0';

    insert(info);
}

void AssetDB::unregistry(Hash64 hash)
{
    auto it = m_refMapped.find(hash);
    if (it == m_refMapped.end())
        return;

    auto info = m_spaseSet.get(it->second);

    logger::trace("[ AssetDB ] unregister: '0x%lX' lookup: '%s'.", hash, info->path);

    m_spaseSet.destroy(it->second);

    m_refMapped.erase(it);
}

void AssetDB::insert(const AssetInfo& info)
{
    m_spaseSet.insert(info.id, &info);

    m_refMapped.emplace(info.hash, info.id);
}

std::string AssetDB::resolve(Hash64 hash)
{
    auto it = m_refMapped.find(hash);
    if (it == m_refMapped.end())
        throw std::runtime_error(String::format("reference '0x%lX' not found...", hash));

    auto info = m_spaseSet.get(it->second);
    if (info == nullptr)
        throw std::runtime_error(String::format("this assetID is not valid id='%d'.", it->second));

    return AssetResolver::Resolve(URI::parse(info->path));
}


