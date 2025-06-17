#include "gr-base/core/task/shadermanager.hpp"

#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/core/thread/taskmanager.hpp"
#include <mutex>
#include <stdexcept>
#include <string>

ShaderManager::ShaderManager()
{}

ShaderManager::~ShaderManager()
{}

grr::gShader *ShaderManager::create(ShaderParams shaderParams)
{
    int id = 0;

    if (shaderParams.flags & ShaderOptimize_static)
    {
        id |= ShaderOptimize_static;
    } else if (shaderParams.flags & ShaderOptimize_dynamic)
    {
        id |= ShaderOptimize_dynamic;
    }

    if (shaderParams.flags & ShaderOptimize_instanced)
    {
        id |= ShaderOptimize_instanced;
    }

    if (shaderParams.flags & ShaderOptimize_animated)
    {
        id |= ShaderOptimize_animated;
    }

    auto name = shaderParams.name + '_' + std::to_string(static_cast<int>(shaderParams.type)) + '_' + std::to_string(id);

    std::unique_lock<std::mutex> lock(s_Mutex);
    {
        auto it = s_ShaderMap.find(name);
        if (it != s_ShaderMap.end())
        {
            std::cout << "Get: " << name << std::endl;
            return it->second.get();
        }
    }

    std::cout << "Create: " << name << std::endl;

    s_ShaderMap.emplace(name, std::make_unique<grr::gShader>());

    return s_ShaderMap[name].get();
}

void ShaderManager::destroy(const grr::gShader *shader)
{
    throw std::runtime_error("Falta corrigir aqui: 55 em ShaderManager.cpp");
}

const char *ShaderManager::get_shader_name(const grr::gShader *shader)
{
    return nullptr;
}

std::shared_ptr<TaskInfo> ShaderManager::create_shader_task_load(ShaderParams shaderParams, const std::string &path, grr::gShader *shader, std::function<void(grr::gShader *)> func)
{
    int id = 0;

    if (shaderParams.flags & ShaderOptimize_static)
    {
        id |= ShaderOptimize_static;
    } else if (shaderParams.flags & ShaderOptimize_dynamic)
    {
        id |= ShaderOptimize_dynamic;
    }

    if (shaderParams.flags & ShaderOptimize_instanced)
    {
        id |= ShaderOptimize_instanced;
    }

    if (shaderParams.flags & ShaderOptimize_animated)
    {
        id |= ShaderOptimize_animated;
    }

    auto name = shaderParams.name + '_' + std::to_string(static_cast<int>(shaderParams.type)) + '_' + std::to_string(id);

    std::unique_lock<std::mutex> lock(s_Mutex);

    // Verifica se já existe uma task de caregamento, se ouver adiciona a func em func list
    {
        auto it = s_ShaderTaskMap.find(name);
        if (it != s_ShaderTaskMap.end())
        {
            if (func == nullptr)
            {
                return it->second;
            }

            auto shaderLoadTask = dynamic_cast<ShaderLoadTask *>(it->second->task.get());

            lock.unlock();
            shaderLoadTask->addFunction(func);

            return it->second;
        }
    }

    { // Verifica se já foi carregado e está na lista principal
        auto it = s_ShaderMap.find(name);
        if (it != s_ShaderMap.end())
        {
            if (func == nullptr)
            {
                return nullptr;
            }

            auto shader = it->second.get();

            lock.unlock();
            
            func(shader);

            return nullptr;
        }
    }

    // cria uma nova task
    auto taskInfo = TaskManager::Add<ShaderLoadTask>(shaderParams, path, shader);
    s_ShaderTaskMap.emplace(name, taskInfo);

    auto shaderLoadTask = dynamic_cast<ShaderLoadTask *>(taskInfo->task.get());
    if (func != nullptr)
    {
        shaderLoadTask->addFunction(func);
    }

    return taskInfo;
}

bool ShaderManager::remove_task(const std::string &name)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_ShaderTaskMap.find(name);
    if (it == s_ShaderTaskMap.end())
    {
        return false;
    }
    s_ShaderTaskMap.erase(it);

    return true;
}

/*
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

grr::gShader *ShaderManager::find(ShaderOptimize optmize, const std::string &name)
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

std::vector<grr::gShader *> ShaderManager::get_shader(AssetShader_ type, ShaderOptimize_ optmize)
{
    std::lock_guard<std::mutex> lock(mutex);
    return ShaderList[type][optmize];
}

std::vector<grr::gShader *> ShaderManager::get_shader(AssetShader_ type)
{
    std::lock_guard<std::mutex> lock(mutex);
    return ShaderListFromType[type];
}

std::shared_ptr<TaskInfo> ShaderManager::create_shader_task(ShaderOptimize_ optmize, const std::string &path, grr::gShader *shader, std::function<void(grr::gShader *)> func)
{
    std::unique_lock<std::mutex> lock(mutex);
    std::cout << "Create Task: " << path << std::endl;

    // check has task
    {
        auto it = ShaderTaskMap.find(path);
        if (it != ShaderTaskMap.end())
        {
            if (func == nullptr)
                return it->second;

            lock.unlock();

            dynamic_cast<ShaderLoadTask *>(&(*it->second->task))->addFunction(func);

            std::cout << "Task Found: " << path << ", Added func" << std::endl;

            // lock.lock();

            return it->second;
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

                    auto info = TaskManager::Add<ShaderLoadTask>(o, path, shader);
                    auto shaderLoadTask = dynamic_cast<ShaderLoadTask *>(info->task.get());
                    ShaderTaskMap.emplace(path, info);

                    lock.unlock();

                    if (func != nullptr)
                        shaderLoadTask->addFunction(func);

                    return info;
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

                        std::cout << "Task has bee loaded: " << path << std::endl;

                        func(shader);

                        return nullptr;
                    }
                }

                std::cout << "GOOOOOOO" << std::endl;
                return nullptr;
            }
        }
    }

    std::cout << "Emplace task: " << path << std::endl;

    auto info = TaskManager::Add<ShaderLoadTask>(optmize, path, shader);
    ShaderTaskMap.emplace(path, info);
    auto shaderLoadTask = dynamic_cast<ShaderLoadTask *>(info->task.get());
    lock.unlock();

    if (func != nullptr)
    {
        shaderLoadTask->addFunction(func);
    }

    return info;
}

bool ShaderManager::remove_task(const std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = ShaderTaskMap.find(name);
    if (it == ShaderTaskMap.end())
    {
        return false;
    }
    std::cout << "Deleted Task: " << name << std::endl;
    ShaderTaskMap.erase(it);
    return true;
}
*/



