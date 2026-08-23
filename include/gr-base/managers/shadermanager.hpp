#pragma once

#include <gr-render/default.hpp>

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    gr::shader *create(const std::string& name);

    void destroy(gr::shader *shader);

    const char *get_shader_name(const gr::shader *shader);

    gr::shader *find(const std::string &name);

    std::mutex s_Mutex;

private:
    std::unordered_map<std::string, std::shared_ptr<gr::shader>> s_ShaderMap;

    std::unordered_map<const gr::shader *, const std::string> s_ShaderNameMap;

    std::vector<gr::shader *> s_ShaderTypeList;
};


