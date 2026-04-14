#pragma once

#include "gr-base/core/file/default.hpp"

#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>

class ModelAsset
{
public:
    std::string name;

    bool isStatic;

    std::vector<ModelAsset> childrent;

    std::unordered_map<uint64_t, std::vector<uint8_t>> componentBuffer;

    bool serialize(Stream* stream) const;

    bool deserialize(Stream* stream);
};


