#pragma once

#include "gr-base/asset/assetProvider.hpp"
#include "gr-base/asset/URI.hpp"

#include <memory>
#include <stdexcept>

class AssetResolver
{
public:
    template <typename T, typename... Args>
    static void Registry(Scheme scheme, Args&&...args);

    static std::string Resolve(const URI& uri);

private:
    static AssetResolver& GetInstance();

    std::shared_ptr<IAssetProvider> m_providers[static_cast<int>(Scheme::Count)];

    std::string resolve(const URI& uri);
};


template <typename T, typename... Args>
inline void AssetResolver::Registry(Scheme scheme, Args&&...args)
{
    GetInstance().m_providers[static_cast<int>(scheme)] = std::make_shared<T>(std::forward<Args>(args)...);
}


inline std::string AssetResolver::Resolve(const URI& uri)
{
    return GetInstance().resolve(uri);
}


