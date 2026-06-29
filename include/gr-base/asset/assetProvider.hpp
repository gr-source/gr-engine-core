#pragma once

#include <string>

class IAssetProvider
{
public:
    virtual std::string resolve(const std::string& path) = 0;
    virtual ~IAssetProvider() = default;
};


