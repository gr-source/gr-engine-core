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

    std::tuple<std::string, std::string, std::vector<std::string>, ShaderDomain> ReadingSourceFileShader();

    std::array<std::string, IShaderStage> LoadShaderBase(ShaderDomain domain);
};

