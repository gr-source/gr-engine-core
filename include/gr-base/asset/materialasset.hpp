#pragma once

#include "gr-base/asset/default.hpp"

#include "gr-base/core/file/default.hpp"

#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>

struct MaterialAsset
{
    std::unordered_map<std::string, std::vector<uint8_t>> overrides;

    std::string shader;

    std::string name;

    template <typename T>
    void Set(const std::string& name, const T& data);

    void Set(const std::string& name, const void* data, size_t size);

    template <typename T>
    T *Get(const std::string& name);

    bool serialize(Stream* stream) const;

    bool deserialize(Stream* stream);
};

template <typename T>
inline void MaterialAsset::Set(const std::string& name, const T& data)
{
    Set(name, &data, sizeof(T));
}

template <typename T>
inline T *MaterialAsset::Get(const std::string& name)
{
    auto it = overrides.find(name);
    if (it == overrides.end())
        return nullptr;

    return reinterpret_cast<T*>(it->second.data());
}


