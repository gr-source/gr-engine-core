#include "gr-base/asset/assetResolver.hpp"

AssetResolver& AssetResolver::GetInstance()
{
    static AssetResolver s_AssetResolver;
    return s_AssetResolver;
}

std::string AssetResolver::resolve(const URI& uri)
{
    auto& provider = m_providers[static_cast<int>(uri.scheme)];
    if (provider == nullptr)
        throw std::runtime_error("provider do not registed.");
    return provider->resolve(uri.path);
}

