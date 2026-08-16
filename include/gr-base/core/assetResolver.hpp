#pragma once

#include "gr-base/core/assetProvider.hpp"

#include "gr-base/asset/URI.hpp"

#include <memory>

class AssetResolver
{
public:
    template <typename T, typename... Args>
    static void Registry(Scheme scheme, Args&&...args);

    static std::string Resolve(const std::string& uri);

    static std::string Resolve(const URI& uri);

private:
    static AssetResolver& GetInstance();

    std::shared_ptr<IAssetProvider> m_providers[Scheme::Count];

    std::string resolve(const URI& uri);
};


template <typename T, typename... Args>
inline void AssetResolver::Registry(Scheme scheme, Args&&...args)
{
    GetInstance().m_providers[scheme] = std::make_shared<T>(std::forward<Args>(args)...);
}


