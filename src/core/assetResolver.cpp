#include "gr-base/core/assetResolver.hpp"

#include <stdexcept>

// public
std::string AssetResolver::Resolve(const URI& uri)
{
    return GetInstance().resolve(uri);
}

std::string AssetResolver::Resolve(const std::string& uri)
{
    return GetInstance().resolve(URI::parse(uri));
}

AssetResolver& AssetResolver::GetInstance()
{
    static AssetResolver s_AssetResolver;
    return s_AssetResolver;
}

std::string AssetResolver::resolve(const URI& uri)
{
    auto& provider = m_providers[uri.scheme];
    if (provider == nullptr)
        throw std::runtime_error("provider do not registed.");
    return provider->resolve(uri.path);
}

