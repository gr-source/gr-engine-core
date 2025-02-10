#pragma once

#include "taskStatus.hpp"

#include <gr-render/gCommon.h>
#include <gr-render/gShader.h>

#include <map>

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

using ShaderFunc = std::function<void (grr::gShader *)>;

class ShaderTask
{
public:
    ShaderTask(const std::string &path, ShaderFunc func, grr::gShader *shader = nullptr);

    void addFunction(ShaderFunc func);

private:
    std::vector<ShaderFunc> m_functions;

    grr::gShader *m_shader;

    void invoke(grr::gShader *shader);
    
    void upload(std::string path, ShaderData frag, ShaderData vert);

protected:
    void doInBackground(std::string path);

public:
    static UniformStackInfo getGLSLStackInfo(const std::string &source);

    static std::string getStructName(const std::string &source);

    static void insert_field(const std::string &line, const std::string &type_name, const std::string &name, std::vector<Uniform> &vector, std::map<std::string, std::vector<Uniform>> &struct_map);

    static std::map<ShaderType, ShaderData> ReadShaderOfType(const std::string &path);

    static grr::UniformType ConvertToGType(UniformType type);

    bool buildShader(const std::string &name, grr::gShader *shader, ShaderData frag, ShaderData vert);
};






