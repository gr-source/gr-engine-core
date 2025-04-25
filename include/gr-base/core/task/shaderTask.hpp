#pragma once

#include "shaderProperty.hpp"

#include <gr-math/types.hpp>

#include <gr-render/gCommon.h>
#include <gr-render/gShader.h>

#include <map>

enum AssetShader
{
    AssetShader_None        = 0,
    AssetShader_Shader2D    = 1,
    AssetShader_Shader3DD   = 2, // dynamic
    AssetShader_Shader3DS   = 3, // static
    AssetShader_Shader3DI   = 4, // instanced
    AssetShader_Count
};

constexpr auto IAssetShaderCount = AssetShader_Count;

using ShaderFunc = std::function<void (grr::gShader *)>;

typedef struct
{
    std::string name;

    std::string type_name;

    grm::u32 count;
} ShaderUniform;

typedef struct MacroLists
{
    std::vector<std::string> definedMacros;

    std::vector<std::string> undefinedMacros;

    std::string content;
} MacroLists;

class ShaderTask
{
public:
    ShaderTask(const std::string &path, ShaderFunc func, grr::gShader *shader = nullptr);

    void addFunction(ShaderFunc func);

private:
    std::map<ShaderType, std::vector<ShaderUniform>> mShaderUniformMapToList;

    std::map<ShaderType, std::string> m_shaderSource;

    std::vector<ShaderFunc> m_functions;

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
    static std::unordered_map<std::string, grr::UniformType> mUniformNameToType;

    static std::map<ShaderType, std::string> LoadShaderBase(ShaderUser_ type);

    static bool ValidStr(const std::string &str, std::string *data, int count);

    static std::vector<std::tuple<std::string, std::string>> ReadStructField(const std::string &content);

    static std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> ReadStuct(const std::string &content);

    static bool IsArray(const std::string &content);

    static std::size_t GetArrayCount(const std::string &content, std::string &name);

    static void SetUniformList(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> &structMap, const std::string &name, const std::string &type, std::vector<ShaderUniform> &vector, bool makeArray = false);

    static std::vector<ShaderUniform> GetRegistryField(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> &structMap, std::unordered_map<std::string, std::string> &fieldMap);

    static std::unordered_map<std::string, std::string> ReadUniform(const std::string &content);

    static std::string RemoveCommenter(const std::string &source);

    static std::string process_macro(const std::string &content, const std::unordered_set<std::string> &macro_map);

    void bind_shader(const MacroLists &macroList);

    bool buildShader(const std::string &name, grr::gShader *shader);

    static MacroLists extract_macros_from_string(const std::string &content);
};






