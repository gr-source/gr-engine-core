#include "gr-base/core/task/shadermanager.hpp"

#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/core/thread/taskmanager.hpp"

#include <algorithm>

ShaderManager::ShaderManager()
{}

ShaderManager::~ShaderManager()
{}

grr::gShader *ShaderManager::create(ShaderParams shaderParams)
{
    auto name = GetShaderParamsName(shaderParams);

    std::unique_lock<std::mutex> lock(s_Mutex);
    {
        auto it = s_ShaderMap.find(name);
        if (it != s_ShaderMap.end())
        {
            return it->second.get();
        }
    }

    s_ShaderMap.emplace(name, std::make_unique<grr::gShader>());

    auto shader = s_ShaderMap[name].get();

    s_ShaderNameMap.emplace(shader, name);

    s_ShaderTypeList[static_cast<int>(shaderParams.type)].push_back(shader);

    return shader;
}

void ShaderManager::destroy(grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_ShaderNameMap.find(shader);
    if (it == s_ShaderNameMap.end())
        return;

    auto shaderParams = GetShaderParamsFromName(it->second);
    {
        auto &shaderlist = s_ShaderTypeList[static_cast<int>(shaderParams.type)];
        shaderlist.erase(std::remove(shaderlist.begin(), shaderlist.end(), shader), shaderlist.end());
    }

    s_ShaderMap.erase(it->second);
    s_ShaderNameMap.erase(it);
}

const char *ShaderManager::get_shader_name(const grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_ShaderNameMap.find(shader);
    if (it == s_ShaderNameMap.end())
    {
        return nullptr;
    }
    return it->second.c_str();
}

grr::gShader *ShaderManager::find(const std::string &name)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_ShaderMap.find(name);
    if (it == s_ShaderMap.end())
    {
        return nullptr;
    }
    return it->second.get();
}

std::vector<grr::gShader *> ShaderManager::get_shader(ShaderType type)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_ShaderTypeList[static_cast<int>(type)];
}

std::shared_ptr<TaskInfo> ShaderManager::create_shader_task_load(ShaderParams shaderParams, grr::gShader *shader, std::function<void(grr::gShader *)> func)
{
    auto taskname = GetShaderParamsName(shaderParams);

    std::unique_lock<std::mutex> lock(s_Mutex);

    // Verifica se já existe uma task de caregamento, se ouver adiciona a func em func list
    {
        auto it = s_ShaderTaskMap.find(taskname);
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
        auto it = s_ShaderMap.find(taskname);
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
    auto taskInfo = TaskManager::Add<ShaderLoadTask>(shaderParams, shader);
    s_ShaderTaskMap.emplace(taskname, taskInfo);

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




