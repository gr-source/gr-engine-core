#pragma once

#include "gr-base/asset/shaderasset.hpp"
#include "gr-base/asset/URI.hpp"

#include "gr-base/core/geometry/shaderData.hpp"

#include "gr-base/core/thread/asynctask.hpp"

#include <unordered_set>
#include <vector>
#include <string>
#include <tuple>


class ShaderBuildTask : public IAsyncTask<ShaderAsset>
{
public:
    ShaderBuildTask(const std::string& file, const URI& uri);

private:
    typedef struct
    {
        std::string name;

        std::string variable;

        uint32_t count;
    } ShaderUniformRaw;

    const URI m_uri;

    const std::string m_file;

    void onPreExecute() override;

    ShaderAsset doInBackground() override;

    void onPostExecute(Result<ShaderAsset> result) override;

    bool compile(const ShaderAsset* asset);

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

