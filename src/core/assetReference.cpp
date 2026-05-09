#include "gr-base/core/assetReference.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/logger.hpp"
#include "gr-base/core/string.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

AssetReference* AssetReference::s_AssetReference = nullptr;

void AssetReference::Initialize()
{
    if (s_AssetReference == nullptr)
        s_AssetReference = new AssetReference();

    logger::info("[AssetReference] Initialize.");
}

void AssetReference::Shutdown()
{
    if (s_AssetReference != nullptr)
        delete s_AssetReference;

    s_AssetReference = nullptr;

    logger::info("[AssetReference] Shutdown.");
}

void AssetReference::Load(const std::string &file)
{
    s_AssetReference->load(file);
}

void AssetReference::Save(const std::string &file)
{
    s_AssetReference->save(file);
}

void AssetReference::Registry(const std::string &name, const std::string& file)
{
    s_AssetReference->registry(name, file);
}

const std::string& AssetReference::GetFile(const std::string &name)
{
    return s_AssetReference->getFile(name);
}

// private
void AssetReference::load(const std::string &file)
{
    std::ifstream is(file);
    if (!is.is_open())
        return logger::error("[AssetReference] Fail in to load source '%s'.", strerror(errno));

    logger::info("[AssetReference] loading ref: '%s'.", file.c_str());

    std::string line;
    while (std::getline(is, line))
    {
        std::string name, file;

        std::stringstream iss(line);
        if (!(iss >> name >> file))
            continue;

        registry(name, file);
    }

    is.close();
}

void AssetReference::save(const std::string &file)
{
    std::ofstream os(file);
    for (auto &&[name, file] : m_refMapped)
        os << name<< " " << file << std::endl;
    os.close();
}

void AssetReference::registry(const std::string &name, const std::string& file)
{
    m_refMapped.emplace(name, file);

    logger::trace("[AssetReference] registered: '%s' lookup: '%s'.", name.c_str(), file.c_str());
}

const std::string& AssetReference::getFile(const std::string& name)
{
    auto it = m_refMapped.find(name);
    if (it != m_refMapped.end())
        return it->second;

    throw std::runtime_error(String::format("reference '%s' not found...", name.c_str()));
}


