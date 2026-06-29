#include "gr-base/asset/assetPtr.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include <gr-render/gTexture.h>

template <>
gr::Texture* AssetPtr<gr::Texture>::Get()
{
    return AssetsManager::GetTexture(handle);
}

template <>
gr::Texture* AssetPtr<gr::Texture>::operator->()
{
    return Get();
}


