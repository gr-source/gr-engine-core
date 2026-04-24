#pragma once

#include "asset.hpp"
#include <unordered_map>

class ShaderAsset : public Asset
{
public:

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

    std::unordered_map<std::string, std::tuple<std::string, int>> m_parameters;
};

