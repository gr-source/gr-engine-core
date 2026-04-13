#pragma once

#include "gr-base/core/file/default.hpp"

#include <unordered_map>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

class MaterialAsset
{
public:
    template <typename T>
    void Set(const std::string& name, const T& data)
    {
        Set(name, &data, sizeof(T));
    }

    template <typename T>
    T *Get(const std::string& name)
    {
        auto it = m_overrides.find(name);
        if (it == m_overrides.end())
            return nullptr;

        return reinterpret_cast<T*>(it->second.data());
    }

    bool serialize(Stream* stream) const;

    bool deserialize(Stream* stream);

private:
    std::unordered_map<std::string, std::vector<uint8_t>> m_overrides;

    void Set(const std::string& name, const void* data, size_t size)
    {
        auto& cache = m_overrides[name];
        cache.resize(size, 0);

        memcpy(cache.data(), data, size);
    }
};



