#pragma once

#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/core/task/shaderTask.hpp"

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

    Shader *create(AssetShader type, ShaderOptimize_ optmize, const std::string &name);

    Shader *find(AssetShader type, ShaderOptimize optmize, const std::string &name) const;

    const char *get_shader_name(const Shader *shader);

    std::vector<Shader *> get_shader(AssetShader type, ShaderOptimize_ optmize) const;

    void create_shader_task(ShaderOptimize_ optmize, const std::string &path, Shader *shader, std::function<void(grr::gShader *)> func);

    void remove_task(const std::string &name);

    static std::mutex mutex;
private:

protected:
    std::unordered_map<std::string, std::array<std::unique_ptr<Shader>, IShaderOptmize>> ShaderMap[IAssetShaderCount];

    std::unordered_map<const Shader *, std::string> ShaderNameMap;

    std::vector<Shader *> ShaderList[IAssetShaderCount][IShaderOptmize];

    std::unordered_map<std::string, std::unique_ptr<ShaderTask>> ShaderTaskMap;

    std::unordered_map<std::string, AssetShader_> ShaderTypeMap;
};


