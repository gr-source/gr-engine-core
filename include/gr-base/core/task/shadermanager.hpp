#pragma once

#include "gr-base/core/task/runtime/shaderloadtask.hpp"

#include "gr-base/core/thread/asynctask.hpp"

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    grr::gShader *create(ShaderParams shaderParams);

    void destroy(const grr::gShader *shader);

    const char *get_shader_name(const grr::gShader *shader);

    grr::gShader *find(const std::string &name);

    std::shared_ptr<TaskInfo> create_shader_task_load(ShaderParams shaderParams, const std::string &path, grr::gShader *shader, std::function<void(grr::gShader *)> func);

    bool remove_task(const std::string &name);

    std::mutex s_Mutex;

private:
    std::unordered_map<std::string, std::unique_ptr<grr::gShader>> s_ShaderMap;

    std::unordered_map<std::string, std::shared_ptr<TaskInfo>> s_ShaderTaskMap;

    /*

    grr::gShader *create(AssetShader type, ShaderOptimize_ optmize, const std::string &name);

    void destroy(grr::gShader *shader);

    grr::gShader *find(ShaderOptimize optmize, const std::string &name);

    const char *get_shader_name(const grr::gShader *shader);

    std::vector<grr::gShader *> get_shader(AssetShader_ type, ShaderOptimize_ optmize);

    std::vector<grr::gShader *> get_shader(AssetShader_ type);

    std::shared_ptr<TaskInfo> create_shader_task(ShaderOptimize_ optmize, const std::string &path, grr::gShader *shader, std::function<void(grr::gShader *)> func);

    bool remove_task(const std::string &name);

    std::mutex mutex;
 
protected:
    std::unordered_map<std::string, std::array<std::unique_ptr<grr::gShader>, IShaderOptmize>> ShaderMap[IAssetShaderCount];

    std::unordered_map<const grr::gShader *, std::string> ShaderNameMap;

    std::vector<grr::gShader *> ShaderList[IAssetShaderCount][IShaderOptmize];

    std::unordered_map<std::string, std::shared_ptr<TaskInfo>> ShaderTaskMap;

    std::unordered_map<std::string, AssetShader_> ShaderTypeMap;

    std::vector<grr::gShader *> ShaderListFromType[IAssetShaderCount];

    std::unordered_map<grr::gShader *, std::tuple<AssetShader_, ShaderOptimize_>> ShaderInfo;
    */
};


