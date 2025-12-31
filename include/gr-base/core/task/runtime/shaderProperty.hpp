#pragma once

#include <gr-render/gCommon.h>
#include <gr-math/gmath.hpp>

#include <unordered_set>

using ShaderStage_ = std::uint8_t;

typedef struct
{
    enum : ShaderStage_
    {
        FRAGMENT    = 0,
        VERTEX      = 1,
        COUNT
    };
} ShaderStage;


constexpr std::uint8_t IShaderStage = static_cast<std::uint8_t>(ShaderStage::COUNT);

struct Uniform {
    std::string name;
    std::string variablename;
    UniformVariable variable;
    u8 count;
};

struct ShaderData {
    std::string source;

    std::vector<Uniform> uniformList;
};

struct UniformStackInfo {
    std::string name;
    std::string object;
    int count;
};

class ShaderProperty
{
public:
    ShaderProperty();

    void addProperty(const std::string &object, const std::string &name, UniformVariable_ variable);

    const std::vector<std::tuple<std::string, std::string, UniformVariable_>> &getProperty() const;

    void clear();

private:
    std::vector<std::tuple<std::string, std::string, UniformVariable_>> mUniformList;
    
    std::unordered_set<std::string> m_tabHash;
};




