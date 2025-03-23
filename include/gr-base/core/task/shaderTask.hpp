#pragma once

#include "shaderProperty.hpp"

#include <gr-math/types.hpp>
#include <gr-render/gCommon.h>
#include <gr-render/gShader.h>

#include <map>

using ShaderFunc = std::function<void (grr::gShader *)>;

typedef struct
{
    std::string name;

    std::string type_name;

    grm::u32 count;
} ShaderUniform;

class ShaderTask
{
public:
    ShaderTask(const std::string &path, ShaderFunc func, grr::gShader *shader = nullptr);

    void addFunction(ShaderFunc func);

private:
    std::map<ShaderType, std::vector<ShaderUniform>> mShaderUniformMapToList;

    std::map<ShaderType, std::string> m_shaderSource;

    std::vector<ShaderFunc> m_functions;

    grr::gShader *m_shader;

    void invoke(grr::gShader *shader);
    
    void upload(std::string path);

protected:
    void doInBackground(std::string path);

public:
    static std::unordered_map<std::string, grr::UniformType> mUniformNameToType;

    static std::map<ShaderType, std::string> LoadShaderBase(ShaderUser type);

    static bool ValidStr(const std::string &str, std::string *data, int count);

    static std::vector<std::tuple<std::string, std::string>> ReadStructField(const std::string &content);

    static std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> ReadStuct(const std::string &content);

    static bool IsArray(const std::string &content);

    static std::size_t GetArrayCount(const std::string &content, std::string &name);

    static void SetUniformList(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> &structMap, const std::string &name, const std::string &type, std::vector<ShaderUniform> &vector, bool makeArray = false);

    static std::vector<ShaderUniform> GetRegistryField(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> &structMap, std::unordered_map<std::string, std::string> &fieldMap);

    static std::unordered_map<std::string, std::string> ReadUniform(const std::string &content);

    static std::string RemoveCommenter(const std::string &source);

    void bindShader(const std::string &path);

    bool buildShader(const std::string &name, grr::gShader *shader);
};






