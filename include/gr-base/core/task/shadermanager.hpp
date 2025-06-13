#pragma once

#include "gr-base/core/task/runtime/shaderloadtask.hpp"

#include <array>
#include <unordered_map>
#include <memory>
#include <string>
#include <mutex>
#include <unordered_set>
#include <vector>

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    grr::gShader *create(AssetShader type, ShaderOptimize_ optmize, const std::string &name);

    void destroy(grr::gShader *shader);

    grr::gShader *find(ShaderOptimize optmize, const std::string &name) const;

    const char *get_shader_name(const grr::gShader *shader);

    std::vector<grr::gShader *> get_shader(AssetShader_ type, ShaderOptimize_ optmize) const;

    std::vector<grr::gShader *> get_shader(AssetShader_ type) const;

    void create_shader_task(ShaderOptimize_ optmize, const std::string &path, grr::gShader *shader, std::function<void(grr::gShader *)> func);

    bool remove_task(const std::string &name);

    static std::mutex mutex;
private:

protected:
    std::unordered_map<std::string, std::array<std::unique_ptr<grr::gShader>, IShaderOptmize>> ShaderMap[IAssetShaderCount];

    std::unordered_map<const grr::gShader *, std::string> ShaderNameMap;

    std::vector<grr::gShader *> ShaderList[IAssetShaderCount][IShaderOptmize];

    std::unordered_map<std::string, std::shared_ptr<AsyncTask>> ShaderTaskMap;

    std::unordered_map<std::string, AssetShader_> ShaderTypeMap;

    std::vector<grr::gShader *> ShaderListFromType[IAssetShaderCount];

    std::unordered_map<grr::gShader *, std::tuple<AssetShader_, ShaderOptimize_>> ShaderInfo;
};


