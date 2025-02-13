#pragma once

#include "shaderProperty.hpp"

#include <gr-render/gCommon.h>
#include <gr-render/gShader.h>

#include <map>

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

    static std::vector<std::string> GetMoreFieldInLine(const std::string &line);
    
    static std::map<ShaderType, ShaderData> ReadShaderOfType(const std::string &path);

    static grr::UniformType ConvertToGType(UniformType type);

    bool buildShader(const std::string &name, grr::gShader *shader, ShaderData frag, ShaderData vert);
};






