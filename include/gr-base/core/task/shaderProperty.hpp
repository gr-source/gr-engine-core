#pragma once

#include "taskStatus.hpp"

#include <gr-render/gCommon.h>
#include <gr-math/gmath.hpp>

#include <unordered_set>

using ShaderUser_ = std::uint8_t;

enum ShaderUser : ShaderUser_
{
    ShaderUser_none     = 1 << 0,
    ShaderUser_shader3D = 1 << 1,
    ShaderUser_shader2D = 1 << 2
};

enum ShaderType {
    ShaderType_None     = 1 << 0,
    ShaderType_Fragment = 1 << 1,
    ShaderType_Vertex   = 1 << 2,
};

struct Uniform {
    std::string name;
    std::string type_name;
    grr::UniformType type;
    grm::u8 count;
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

    void addProperty(const std::string &object, const std::string &name, grr::UniformType type);

    const std::vector<std::tuple<std::string, std::string, grr::UniformType>> &getProperty() const;

    void clear();

private:
    std::vector<std::tuple<std::string, std::string, grr::UniformType>> mUniformList;
    
    std::unordered_set<std::string> m_tabHash;
};




