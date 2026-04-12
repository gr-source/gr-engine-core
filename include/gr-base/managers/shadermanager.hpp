#pragma once

#include "gr-base/core/task/runtime/shaderloadtask.hpp"

#include "gr-base/core/thread/asynctask.hpp"

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    gr::Shader *create(ShaderParams shaderParams);

    void destroy(gr::Shader *shader);

    const char *get_shader_name(const gr::Shader *shader);

    gr::Shader *find(const std::string &name);

    std::vector<gr::Shader *>get_shader(ShaderType type);

    std::shared_ptr<TaskInfo> load(ShaderParams shaderParams, gr::Shader *shader, std::function<void(gr::Shader *)> onLoaded);

    bool remove_task(const std::string &name);

    std::mutex s_Mutex;

private:
    std::unordered_map<std::string, std::unique_ptr<gr::Shader>> s_ShaderMap;

    std::unordered_map<const gr::Shader *, const std::string> s_ShaderNameMap;

    std::unordered_map<std::string, std::shared_ptr<TaskInfo>> s_ShaderTaskMap;

    std::array<std::vector<gr::Shader *>, ShaderTypeCount> s_ShaderTypeList;
};


