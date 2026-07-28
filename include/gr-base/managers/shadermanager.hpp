#pragma once

#include <gr-render/gCommon.h>

#include "gr-base/core/geometry/shaderData.hpp"
#include "gr-base/core/thread/asynctask.hpp"

#include <unordered_map>
#include <vector>

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    gr::shader *create(const std::string& name);

    void destroy(gr::shader *shader);

    const char *get_shader_name(const gr::shader *shader);

    gr::shader *find(const std::string &name);

    std::vector<gr::shader *>get_shader(ShaderType type);

    std::mutex s_Mutex;

private:
    std::unordered_map<std::string, std::shared_ptr<gr::shader>> s_ShaderMap;

    std::unordered_map<const gr::shader *, const std::string> s_ShaderNameMap;

    std::vector<gr::shader *> s_ShaderTypeList;
};


