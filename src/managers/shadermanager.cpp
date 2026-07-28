#include "gr-base/managers/shadermanager.hpp"

#include "gr-base/core/thread/taskmanager.hpp"

#include <gr-render/shader.hpp>

#include <algorithm>

ShaderManager::ShaderManager()
{}

ShaderManager::~ShaderManager()
{}

gr::shader *ShaderManager::create(const std::string& name)
{
    std::unique_lock<std::mutex> lock(s_Mutex);
    {
        auto it = s_ShaderMap.find(name);
        if (it != s_ShaderMap.end())
        {
            return it->second.get();
        }
    }

    s_ShaderMap.emplace(name, std::make_shared<gr::shader>());

    auto shader = s_ShaderMap[name].get();

    s_ShaderNameMap.emplace(shader, name);

    s_ShaderTypeList.push_back(shader);

    return shader;
}

void ShaderManager::destroy(gr::shader *shader)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_ShaderNameMap.find(shader);
    if (it == s_ShaderNameMap.end())
        return;

    s_ShaderTypeList.erase(std::remove(s_ShaderTypeList.begin(), s_ShaderTypeList.end(), shader), s_ShaderTypeList.end());

    s_ShaderMap.erase(it->second);
    s_ShaderNameMap.erase(it);
}

const char *ShaderManager::get_shader_name(const gr::shader *shader)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_ShaderNameMap.find(shader);
    if (it == s_ShaderNameMap.end())
    {
        return nullptr;
    }
    return it->second.c_str();
}

gr::shader *ShaderManager::find(const std::string &name)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_ShaderMap.find(name);
    if (it == s_ShaderMap.end())
    {
        return nullptr;
    }
    return it->second.get();
}

std::vector<gr::shader *> ShaderManager::get_shader(ShaderType type)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_ShaderTypeList;
}





