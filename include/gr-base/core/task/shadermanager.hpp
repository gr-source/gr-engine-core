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

    std::unordered_map<const grr::gShader *, const std::string> s_ShaderNameMap;

    std::unordered_map<std::string, std::shared_ptr<TaskInfo>> s_ShaderTaskMap;
};


