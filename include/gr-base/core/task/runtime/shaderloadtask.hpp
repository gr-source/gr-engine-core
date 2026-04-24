#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "default.hpp"

#include "gr-base/core/geometry/shaderData.hpp"

#include <unordered_set>

typedef struct
{
    std::string name;

    std::string variable;

    u32 count;
} ShaderUniformRaw;

class ShaderLoadTask : public IAsyncTask<int>
{
public:
    ShaderLoadTask() = delete;
    ShaderLoadTask(ShaderParams shaderParams);

    void addFunction(std::function<void (gr::Shader *)> func);

private:
    std::vector<std::function<void (gr::Shader *)>> m_func_list;

    ShaderParams m_shaderParams;

    const std::string m_path;

    int doInBackground() override;

    void onPostExecute(int result) override;

protected:
    std::vector<ShaderUniformRaw> s_shader_uniform_list;

    // std::unordered_set<std::string> userfield;

    std::array<std::string, IShaderStage> m_shaderSource;

    static ShaderType get_shader_asset_type(const std::string &path);

    static std::unordered_map<std::string, UniformType> mTypeMapping;

    static std::array<std::string, IShaderStage> LoadShaderBase(ShaderType type);

    static bool ValidStr(const std::string &str, std::string *data, int count);

    static std::vector<std::tuple<std::string, std::string, bool>> ReadStructField(const std::string &content);

    static std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> ReadStuct(const std::string &content);

    static bool IsArray(const std::string &content);

    static std::size_t GetArrayCount(const std::string &content, std::string &name);

    static void SetUniformList(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> &structMap, const std::string &name, const std::string &type, std::vector<ShaderUniformRaw> &vector, bool isarray);

    static std::vector<ShaderUniformRaw> GetRegistryField(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> &structMap, std::unordered_map<std::string, std::tuple<std::string, bool>> &fieldMap);

    static std::unordered_map<std::string, std::tuple<std::string, bool>> ReadUniform(const std::string &content);

    static std::string process_macro(const std::string &content, const std::unordered_set<std::string> &macro_map);

    void bind_shader(const std::string &path);

    bool buildShader(const std::string &name, gr::Shader *shader);

    static std::string remove_comment(const std::string &str);
};

/*
using ShaderFunc = std::function<void (gr::gShader *)>;

typedef struct
{
    std::string name;

    std::string variable;

    grm::u32 count;
} ShaderUniform;

class ShaderLoadTask : public AsyncTask
{
public:
    ShaderLoadTask(ShaderOptimize_ optimize, const std::string &path, ShaderFunc func, gr::gShader *shader = nullptr);
    ~ShaderLoadTask();

    void addFunction(ShaderFunc func);

private:
    std::array<std::vector<ShaderUniform>, IShaderStage> mShaderUniformMapToList;

    std::array<std::string, IShaderStage> m_shaderSource;

    std::vector<ShaderFunc> m_functions;

    
    const ShaderOptimize_ shaderoptimize;
    
    AssetShader m_asset_shader_type;

    gr::gShader *m_shader;

    const std::string m_path;

    const bool bShaderCreated;

    void invoke(gr::gShader *shader);
    
    void upload(std::string path);

protected:
    void doInBackground() override;

    // void doInBackground(std::string path);

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

    bool buildShader(const std::string &name, gr::gShader *shader);

    static std::string remove_comment(const std::string &str);
};
*/





