#pragma once

#include "gr-base/prefab/default.hpp"

#include <gr-render/default.hpp>

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <string>

struct Material
{
    std::unordered_map<std::string, std::vector<uint8_t>> overrides;

    std::vector<uint8_t> GPUbuffer;

    ShaderPrefab *prefab;

    gr::shader* lastShader = nullptr;
    gr::shader* shader = nullptr;
    
    template <typename T>
    void Set(const std::string &name, const T &value);

    void Set(const std::string &name, const void *data, size_t size);

    template <typename T>
    const T* Get(const std::string& name) const;

    const void* GetInternal(const std::string &name) const;

    void Sync();

    void Build();

    void Flush() const;
};

template <typename T>
inline const T *Material::Get(const std::string &name) const
{
    return reinterpret_cast<const T*>(GetInternal(name.c_str()));
}

template <typename T>
inline void Material::Set(const std::string &name, const T &value)
{
    Set(name.c_str(), &value, sizeof(T));
}


