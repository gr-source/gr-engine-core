#pragma once

#include "gr-base/prefab/default.hpp"

#include <cstdint>
#include <string>
#include <array>

#include <gr-render/gRender.h>

struct Material
{
    std::unordered_map<std::string, std::vector<uint8_t>> overrides;

    std::vector<uint8_t> buffer;

    ShaderPrefab *prefab;

    gr::Shader* shader = nullptr;
    
    template <typename T>
    void Set(const std::string &name, const T &value);

    void Set(const std::string &name, const void *data, size_t size);

    template <typename T>
    const T* Get(const std::string& name);

    const void* GetInternal(const std::string &name) const;

    void Build();

    void Flush();
};

template <typename T>
inline const T *Material::Get(const std::string &name)
{
    return reinterpret_cast<const T*>(GetInternal(name.c_str()));
}

template <typename T>
inline void Material::Set(const std::string &name, const T &value)
{
    Set(name.c_str(), &value, sizeof(T));
}


