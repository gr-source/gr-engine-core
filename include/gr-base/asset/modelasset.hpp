#pragma once

#include "gr-base/asset/iasset.hpp"

#include "gr-base/ecs/type_traits.hpp"

#include "gr-base/asset/default.hpp"

#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>

struct ModelAsset : IAsset
{
    std::unordered_map<Hash64, std::vector<uint8_t>> componentBuffer;

    std::vector<ModelAsset> children;

    std::vector<AssetID> materials;

    AssetID mesh = INVALID_ASSET;
    
    std::string name;

    bool isStatic = false;

    template <typename T>
    void AddComponent(const T& pref);

    void AddComponent(Hash64 hash, const void* data, size_t size);

    bool serialize(Stream* stream) const override;

    bool deserialize(Stream* stream) override;
};

template <typename T>
inline void ModelAsset::AddComponent(const T& pref)
{
    AddComponent(engine::type_traits::hash<T>, &pref, sizeof(T));
}

