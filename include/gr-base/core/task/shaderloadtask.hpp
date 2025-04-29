#pragma once

#include "shaderProperty.hpp"

#include <gr-math/types.hpp>

#include <gr-render/gCommon.h>

#include "gr-base/core/geometry/shader.hpp"

#include <unordered_set>

using ShaderFunc = std::function<void (grr::gShader *)>;

typedef struct
{
    std::string name;

    std::string variable;

    grm::u32 count;
} ShaderUniform;

class ShaderLoadTask
{
public:
    ShaderLoadTask(ShaderOptimize_ optimize, const std::string &path, ShaderFunc func, grr::gShader *shader = nullptr);

    void addFunction(ShaderFunc func);

private:
    std::array<std::vector<ShaderUniform>, IShaderStage> mShaderUniformMapToList;

    std::array<std::string, IShaderStage> m_shaderSource;

    std::vector<ShaderFunc> m_functions;

    std::unordered_set<std::string> userfield;
    
    ShaderOptimize_ shaderoptimize;
    
    AssetShader m_asset_shader_type;

    grr::gShader *m_shader;

    std::string task_name;

    bool bShaderCreated;

    void invoke(grr::gShader *shader);
    
    void upload(std::string path);

protected:
    void doInBackground(std::string path);

    static AssetShader get_shader_asset_type(const std::string &path);

public:
    static std::unordered_map<std::string, UniformVariable_> mVariableNameToID;

    static std::array<std::string, IShaderStage> LoadShaderBase(ShaderType_ type);

    static bool ValidStr(const std::string &str, std::string *data, int count);

    static std::vector<std::tuple<std::string, std::string, bool>> ReadStructField(const std::string &content);

    static std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> ReadStuct(const std::string &content);

    static bool IsArray(const std::string &content);

    static std::size_t GetArrayCount(const std::string &content, std::string &name);

    static void SetUniformList(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> &structMap, const std::string &name, const std::string &type, std::vector<ShaderUniform> &vector, bool isarray);

    static std::vector<ShaderUniform> GetRegistryField(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> &structMap, std::unordered_map<std::string, std::tuple<std::string, bool>> &fieldMap);

    static std::unordered_map<std::string, std::tuple<std::string, bool>> ReadUniform(const std::string &content);

    static std::string process_macro(const std::string &content, const std::unordered_set<std::string> &macro_map);

    void bind_shader(const std::string &path);

    bool buildShader(const std::string &name, grr::gShader *shader);

    static std::string remove_comment(const std::string &str);
};






