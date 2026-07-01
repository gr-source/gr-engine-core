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
        std::string file;
        Hash64 hash;

        std::stringstream iss(line);
        if (!(iss >> hash >> file))
            continue;

        registry(hash, file);
    }

    is.close();
}

void AssetDB::save(const std::string &file)
{
    std::ofstream os(file);
    for (auto &&[hash, file] : m_refMapped)
        os << hash << " " << file << std::endl;
    os.close();
}

void AssetDB::registry(Hash64 hash, const std::string& path)
{
    m_refMapped.emplace(hash, path);

    AssetID id = m_nextID++;

    std::cout << "ID: " << id << path << std::endl;

    // logger::trace("[AssetDB] register: '0x%lX' lookup: '%s'.", hash, path.c_str());
}

void AssetDB::unregistry(Hash64 hash)
{
    auto it = m_refMapped.find(hash);
    if (it == m_refMapped.end())
        return;

    logger::trace("[AssetDB] unregister: '0x%lX' lookup: '%s'.", hash, it->second.c_str());

    m_refMapped.erase(it);
}

std::string AssetDB::resolve(Hash64 hash)
{
    auto it = m_refMapped.find(hash);
    if (it != m_refMapped.end())
        return AssetResolver::Resolve(URI::parse(it->second));

    throw std::runtime_error(String::format("reference '0x%lX' not found...", hash));
}


