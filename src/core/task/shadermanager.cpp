#include "gr-base/core/task/shadermanager.hpp"

#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/core/thread/taskmanager.hpp"

#include <array>
#include <cstddef>
#include <mutex>
#include <tuple>

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

grr::gShader *ShaderManager::create(AssetShader type, ShaderOptimize_ optmize, const std::string &name)
{
    std::unique_lock<std::mutex> lock(mutex);

    {
        auto it = ShaderMap[type].find(name);
        if (it == ShaderMap[type].end())
        {
            auto &&[its, added] = ShaderMap[type].emplace(name, std::array<std::unique_ptr<grr::gShader>, IShaderOptmize>());
            for (std::size_t i=0;i<IShaderOptmize;i++)
                its->second[i] = nullptr;
        }
    }
    auto &ShaderOpt = ShaderMap[type][name];

    bool bexists = ShaderOpt[optmize] != nullptr;

    ShaderOpt[optmize] = std::make_unique<grr::gShader>();

    if (!bexists)
    {
        ShaderInfo.emplace(&(*ShaderOpt[optmize]), std::make_tuple(type, optmize));
    }
    
    ShaderList[type][optmize].emplace_back(&(*ShaderOpt[optmize]));
    ShaderListFromType[type].emplace_back(&(*ShaderOpt[optmize]));

    ShaderNameMap.emplace(&(*ShaderOpt[optmize]), name);

    ShaderTypeMap.emplace(name, type);

    return &(*ShaderOpt[optmize]);
}

void ShaderManager::destroy(grr::gShader *shader)
{
    std::cout << "destroy: " << get_shader_name(shader) << std::endl;
}

grr::gShader *ShaderManager::find(ShaderOptimize optmize, const std::string &name) const
{
    std::unique_lock<std::mutex> lock(mutex);
    auto it_type = ShaderTypeMap.find(name);
    if (it_type == ShaderTypeMap.end())
    {
        return nullptr;
    }

    auto it = ShaderMap[it_type->second].find(name);
    if (it == ShaderMap[it_type->second].end())
    {
        return nullptr;
    }

    if (it->second[optmize] == nullptr)
    {
        return nullptr;
    }

    return &(*it->second[optmize]);
}

const char *ShaderManager::get_shader_name(const grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = ShaderNameMap.find(shader);
    if (it == ShaderNameMap.end())
    {
        return nullptr;
    }
    return it->second.c_str();
}

std::vector<grr::gShader *> ShaderManager::get_shader(AssetShader_ type, ShaderOptimize_ optmize) const
{
    std::lock_guard<std::mutex> lock(mutex);
    return ShaderList[type][optmize];
}

std::vector<grr::gShader *> ShaderManager::get_shader(AssetShader_ type) const
{
    std::lock_guard<std::mutex> lock(mutex);
    return ShaderListFromType[type];
}

void ShaderManager::create_shader_task(ShaderOptimize_ optmize, const std::string &path, grr::gShader *shader, std::function<void(grr::gShader *)> func)
{
    std::unique_lock<std::mutex> lock(mutex);

    // check has task
    {
        auto it = ShaderTaskMap.find(path);
        if (it != ShaderTaskMap.end())
        {
            if (func == nullptr)
                return;

            lock.unlock();

            dynamic_cast<ShaderLoadTask *>(&(*it->second))->addFunction(func);

            lock.lock();

            return;
        }
    }

    // check shader data
    {
        auto it = ShaderTypeMap.find(path);
        if (it != ShaderTypeMap.end())
        {
            if (shader != nullptr)
            {
                auto itsw = ShaderInfo.find(shader);
                if (itsw != ShaderInfo.end())
                {
                    auto &&[t, o] = itsw->second;

                    lock.lock();

                    auto info = TaskManager::Add<ShaderLoadTask>(o, path, func, shader);

                    lock.unlock();

                    ShaderTaskMap.emplace(path, info->task);
                    return;
                }
            } else
            {
                auto its = ShaderMap[it->second].find(path);
                if (its != ShaderMap[it->second].end())
                {
                    if (func != nullptr && its->second[optmize] != nullptr)
                    {
                        auto shader = &(*its->second[optmize]);

                        lock.unlock();

                        return func(shader);
                    }
                }
            }
        }
    }

    lock.unlock();
    auto info = TaskManager::Add<ShaderLoadTask>(optmize, path, func, shader);
    lock.lock();

    ShaderTaskMap.emplace(path, info->task);
}

bool ShaderManager::remove_task(const std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = ShaderTaskMap.find(name);
    if (it == ShaderTaskMap.end())
    {
        return false;
    }
    ShaderTaskMap.erase(it);
    return true;
}

