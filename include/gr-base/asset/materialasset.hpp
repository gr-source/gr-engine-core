#pragma once

#include "gr-base/asset/asset.hpp"
#include "gr-base/core/default.hpp"

#include <unordered_map>
#include <cstdint>
#include <vector>

class MaterialAsset
{
public:
    std::string name;

    AssetHash hash = INVALID_HASH64;

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

    inline const std::unordered_map<std::string, std::vector<uint8_t>>& GetOverrides() const
    {
        return m_overrides;
    }

    inline void SetShader(const std::string& name)
    {
        m_shader = name;
    }

    inline const std::string& GetShader() const
    {
        return m_shader;
    }

    bool serialize(Stream* stream) const;

    bool deserialize(Stream* stream);

private:
    std::unordered_map<std::string, std::vector<uint8_t>> m_overrides;

    std::string m_shader;

    void Set(const std::string& name, const void* data, size_t size);
};



