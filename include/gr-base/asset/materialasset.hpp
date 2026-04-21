#pragma once

#include "gr-base/asset/asset.hpp"

#include <unordered_map>
#include <cstdint>
#include <vector>

class MaterialAsset : public Asset
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

    inline void SetShader(const std::string& name)
    {
        m_shader = name;
    }

    inline const std::string& GetShader() const
    {
        return m_shader;
    }

    inline void SetName(const std::string& name)
    {
        m_name = name;
    }

    inline const std::string& GetName() const
    {
        return m_name;
    }

    bool serialize(Stream* stream) const override;

    bool deserialize(Stream* stream) override;

private:
    std::unordered_map<std::string, std::vector<uint8_t>> m_overrides;

    std::string m_shader;

    std::string m_name;

    void Set(const std::string& name, const void* data, size_t size);
};



