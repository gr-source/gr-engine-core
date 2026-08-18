#include "gr-base/core/assetResolver.hpp"

#include <stdexcept>

AssetResolver* AssetResolver::gAssetResolver =
    nullptr;

// public
void AssetResolver::Initialize()
{
    if (gAssetResolver == nullptr)
        gAssetResolver = new AssetResolver();
}

void AssetResolver::Shutdown()
{
    if (gAssetResolver != nullptr)
        delete gAssetResolver;
    gAssetResolver = nullptr;
}

std::string AssetResolver::Resolve(const URI& uri)
{
    return gAssetResolver->resolve(uri);
}

std::string AssetResolver::Resolve(const std::string& uri)
{
    return gAssetResolver->resolve(URI::parse(uri));
}

std::string AssetResolver::resolve(const URI& uri)
{
    auto& provider = m_providers[uri.scheme];
    if (provider == nullptr)
        throw std::runtime_error("provider do not registed.");
    return provider->resolve(uri.path);
}

