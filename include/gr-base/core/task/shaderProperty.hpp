#pragma once

#include "taskStatus.hpp"

enum ShaderType {
    ShaderType_None     = 1 << 0,
    ShaderType_Fragment = 1 << 1,
    ShaderType_Vertex   = 1 << 2,
};

enum UniformType {
    UniformType_object    = 1 << 0,
    UniformType_float     = 1 << 1,
    UniformType_int       = 1 << 2,
    UniformType_bool      = 1 << 3,
    UniformType_mat3      = 1 << 4,
    UniformType_mat4      = 1 << 5,
    UniformType_vec2      = 1 << 6,
    UniformType_vec3      = 1 << 7,
    UniformType_vec4      = 1 << 8,
    UniformType_sampler2D = 1 << 9,
    UniformType_samplerCube = 1 << 10,
};

struct Uniform {
    std::string name;
    std::string type_name;
    UniformType type;
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

    void addProperty(const std::string &object, const std::string &name, UniformType type);

    const std::vector<std::tuple<std::string, std::string, UniformType>> &getProperty() const;

    void clear();

private:
    std::vector<std::tuple<std::string, std::string, UniformType>> mUniformList;
};




