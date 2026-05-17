#pragma once

#include "gr-base/asset/shaderasset.hpp"
#include "gr-base/core/geometry/shaderData.hpp"

#include "gr-base/core/task/runtime/shaderloadtask.hpp"
#include "gr-base/core/thread/asynctask.hpp"

#include <memory>
#include <string>
#include <tuple>
#include <unordered_set>

class ShaderBuildTask : public IAsyncTask<int>
{
public:
    ShaderBuildTask(const std::string& file, const std::string& source);

private:
    std::string m_file;

    std::string m_source;

    std::vector<std::tuple<std::string, std::string>> m_shaderVariants;

    void onPreExecute() override;

    int doInBackground() override;

    void onPostExecute(int result) override;

    ShaderAsset onStartParceShader();

    std::tuple<std::string, std::string, std::vector<std::string>, ShaderType> ReadingSourceFileShader();

    std::array<std::string, IShaderStage> LoadShaderBase(ShaderType type);

    std::string remove_comment(const std::string &str);

    std::string process_macro(const std::string &source, const std::unordered_set<std::string> &defines);

    std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> ReadStuct(const std::string &content);

    bool ValidStr(const std::string &str, std::string *data, int count);

    std::vector<std::tuple<std::string, std::string, bool>> ReadStructField(const std::string &content);

    std::unordered_map<std::string, std::tuple<std::string, bool>> ReadUniform(const std::string &content);

    std::vector<ShaderUniformRaw> GetRegistryField(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> &structMap, std::unordered_map<std::string, std::tuple<std::string, bool>> &uniformMap);

    void SetUniformList(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> &structMap, const std::string &name, const std::string &type, std::vector<ShaderUniformRaw> &vector, bool isarray);

    std::size_t GetArrayCount(const std::string &content, std::string &name);
};

