#pragma once

#include "gr-base/core/file/default.hpp"

#include "gr-base/ecs/type_traits.hpp"

#include "gr-base/asset/default.hpp"

#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>

struct ModelAsset
{
    std::unordered_map<Hash64, std::vector<uint8_t>> componentBuffer;

    std::vector<ModelAsset> childrent;

    std::vector<AssetHash> materials;
    
    std::string name;

    AssetHash mesh = INVALID_HASH64;

    bool isStatic = false;

    template <typename T>
    inline void AddComponent(const T& pref)
    {
        AddComponent(engine::type_traits::hash<T>, &pref, sizeof(T));
    }

    void AddComponent(Hash64 hash, const void* data, size_t size);

    bool serialize(Stream* stream) const;

    bool deserialize(Stream* stream);
};


