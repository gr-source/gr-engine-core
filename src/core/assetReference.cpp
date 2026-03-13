#include "gr-base/core/assetReference.hpp"
#include "gr-base/core/console.hpp"

#include <cstring>
#include <fstream>
#include <iostream>

AssetReference* AssetReference::s_AssetReference = nullptr;

void AssetReference::Initialize()
{
    if (s_AssetReference == nullptr)
        s_AssetReference = new AssetReference();
}

void AssetReference::Shutdown()
{
    if (s_AssetReference != nullptr)
        delete s_AssetReference;

    s_AssetReference = nullptr;
}

void AssetReference::Load(const std::string &file)
{
    s_AssetReference->load(file);
}

void AssetReference::Save(const std::string &file)
{
    s_AssetReference->save(file);
}

void AssetReference::Registry(const std::string &name)
{
    s_AssetReference->registry(name);
}

std::string AssetReference::Get(AssetHash hash)
{
    return s_AssetReference->get(hash);
}

// private
void AssetReference::load(const std::string &file)
{
    std::ifstream is(file);
    if (!is.is_open())
    {
        return Console::LogWarning("[AssetReference] %s; %s", __FUNCTION__, strerror(errno));
    }

    std::string line;
    while (std::getline(is, line))
        registry(line);

    is.close();
}

void AssetReference::save(const std::string &file)
{
    std::ofstream os(file);
    for (auto &&[name, hash] : m_refMapped)
        os << name << std::endl;
    os.close();
}

void AssetReference::registry(const std::string &name)
{
    AssetHash hash = GetHash(name.c_str());

    m_refMapped.emplace(name, hash);

    m_refHashMapped.emplace(hash, name);
}

std::string AssetReference::get(AssetHash hash)
{
    auto it = m_refHashMapped.find(hash);
    if (it != m_refHashMapped.end())
        return it->second;
    return "";
}


