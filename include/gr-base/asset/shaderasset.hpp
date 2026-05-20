#pragma once

#include "asset.hpp"
#include <cstdint>
#include <unordered_map>

typedef uint32_t ShaderAssetFlags_;

enum : uint32_t
{
    ShaderAssetFlags_Shader3D   = 1 << 1,
    ShaderAssetFlags_Shader2D   = 1 << 2,
    ShaderAssetFlags_Line       = 1 << 3
};

class ShaderAsset : public Asset
{
public:
    ShaderAsset();

    inline void SetName(const std::string& name)
    {
        m_name = name;
    }

    inline const std::string& GetName() const
    {
        return m_name;
    }

    inline void SetFragment(const std::string& src)
    {
        m_fragment = src;
    }

    inline const std::string& GetFragment() const
    {
        return m_fragment;
    }

    inline void SetVertex(const std::string& src)
    {
        m_vertex = src;
    }

    inline const std::string& GetVertex() const
    {
        return m_vertex;
    }

    inline void SetFlags(uint32_t flags)
    {
        m_flags = flags;
    }

    inline uint32_t GetFlags() const
    {
        return m_flags;
    }

    void AddUniform(const std::string& name, const std::string& typesname, int count);

    inline const std::unordered_map<std::string, std::tuple<std::string, int>>& GetUnifoms() const
    {
        return m_parameters;
    }

    bool serialize(Stream* stream) const override;

    bool deserialize(Stream* stream) override;

private:
    std::string m_name;

    std::string m_fragment;

    std::string m_vertex;

    uint32_t m_flags;

    std::unordered_map<std::string, std::tuple<std::string, int>> m_parameters;
};

