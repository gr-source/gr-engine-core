#pragma once

#include <gr-render/gCommon.h>

#include "gr-base/core/geometry/shaderData.hpp"
#include "gr-base/core/thread/asynctask.hpp"

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    gr::Shader *create(const std::string& name);

    void destroy(gr::Shader *shader);

    const char *get_shader_name(const gr::Shader *shader);

    gr::Shader *find(const std::string &name);

    std::vector<gr::Shader *>get_shader(ShaderType type);

    std::mutex s_Mutex;

private:
    std::unordered_map<std::string, std::unique_ptr<gr::Shader>> s_ShaderMap;

    std::unordered_map<const gr::Shader *, const std::string> s_ShaderNameMap;

    std::vector<gr::Shader *> s_ShaderTypeList;
};


