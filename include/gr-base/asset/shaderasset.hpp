#pragma once

#include "asset.hpp"

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

private:
    std::string m_name;
};

