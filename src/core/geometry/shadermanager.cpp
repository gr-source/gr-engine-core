#include "gr-base/core/geometry/shadermanager.hpp"
#include "gr-base/core/geometry/shader.hpp"
#include <array>
#include <cstddef>
#include <mutex>

std::mutex ShaderManager::mutex;

ShaderManager::ShaderManager()
{}

ShaderManager::~ShaderManager()
{
}

std::array<const char *, IAssetShaderCount> AssetShaderName = {
    "None",
    "Shader2D",
    "Shader3D"
};

std::array<const char *, IShaderOptmize> ShaderOptimizeName = {
    "None",
    "DYNAMIC",
    "STATIC",
    "INSTANCED_DYNAMIC",
    "INSTANCED_STATIC",
    "ANIMATION_DYNAMIC",
    "ANIMATION_STATIC",
    "ANIMATION_INSTANCED",
};

Shader *ShaderManager::create(AssetShader type, ShaderOptimize_ optmize, const std::string &name)
{
    std::cout << name << ": " << AssetShaderName[type] << ", " << ShaderOptimizeName[optmize] << std::endl;

    std::lock_guard<std::mutex> lock(mutex);

    {
        auto it = ShaderMap[type].find(name);
        if (it == ShaderMap[type].end())
        {
            auto &&[its, added] = ShaderMap[type].emplace(name, std::array<std::unique_ptr<Shader>, IShaderOptmize>());
            for (std::size_t i=0;i<IShaderOptmize;i++)
                its->second[i] = nullptr;
        }
    }
    auto &ShaderOpt = ShaderMap[type][name];

    ShaderOpt[optmize] = std::make_unique<Shader>(type, optmize);
    
    ShaderList[type][optmize].emplace_back(&(*ShaderOpt[optmize]));

    ShaderNameMap.emplace(&(*ShaderOpt[optmize]), name);

    ShaderTypeMap.emplace(name, type);

    return &(*ShaderOpt[optmize]);
}

Shader *ShaderManager::find(AssetShader type, ShaderOptimize optmize, const std::string &name) const
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = ShaderMap[type].find(name);
    if (it == ShaderMap[type].end())
    {
        return nullptr;
    }
    return &(*it->second[optmize]);
}

const char *ShaderManager::get_shader_name(const Shader *shader)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = ShaderNameMap.find(shader);
    if (it == ShaderNameMap.end())
    {
        return nullptr;
    }
    return it->second.c_str();
}

std::vector<Shader *> ShaderManager::get_shader(AssetShader type, ShaderOptimize_ optmize) const
{
    std::lock_guard<std::mutex> lock(mutex);
    return ShaderList[type][optmize];
}

void ShaderManager::create_shader_task(ShaderOptimize_ optmize, const std::string &path, Shader *shader, std::function<void(grr::gShader *)> func)
{
    std::lock_guard<std::mutex> lock(mutex);
    // check has task
    {
        auto it = ShaderTaskMap.find(path);
        if (it != ShaderTaskMap.end())
        {
            if (func == nullptr)
            {
                return;
            }

            return it->second->addFunction(func);
        }
    }

    // check shader data
    {
        auto it = ShaderTypeMap.find(path);
        if (it != ShaderTypeMap.end())
        {
            auto its = ShaderMap[it->second].find(path);
            if (its != ShaderMap[it->second].end() && shader == nullptr)
            {
                if (func != nullptr && its->second[optmize] != nullptr)
                    return func(&(*its->second[optmize]));
            }
        }
    }

    ShaderTaskMap.emplace(path, std::make_unique<ShaderTask>(optmize, path, func, shader));
}

void ShaderManager::remove_task(const std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = ShaderTaskMap.find(name);
    if (it == ShaderTaskMap.end())
    {
        return;
    }

    ShaderTaskMap.erase(it);
}

