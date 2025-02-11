#include "gr-base/core/task/shaderTask.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/event.hpp"

#include <gr-render/gShader.h>
#include <gr-render/gError.h>

#include <filesystem>
#include <fstream>

#define PRINT_ERROR(func) \
    if (func == -1) { \
        Console::LogError("[ ShaderTask ] Shader.Regiter uniform: %s", grr::error()); \
    }

ShaderTask::ShaderTask(const std::string &path, ShaderFunc func, grr::gShader *shader) : m_shader(shader)
{
    if (func != nullptr)
        m_functions.push_back(func);

    Threadpool::AddTask(&ShaderTask::doInBackground, this, path);
}

void ShaderTask::addFunction(ShaderFunc func)
{
    m_functions.push_back(func);
}

void ShaderTask::invoke(grr::gShader *shader)
{
    for (auto func : m_functions)
        func(shader);
}

void ShaderTask::upload(std::string path, ShaderData frag, ShaderData vert)
{
    auto shader = m_shader != nullptr ? m_shader : AssetsManager::CreateShader(path);

    auto name = std::filesystem::path(path).filename().string();
    if (!buildShader(name, shader, frag, vert)) {
        return AssetsManager::RemoveTask(path);
    }

    invoke(shader);

    Console::LogInfo("[ ShaderTask ] Shader loaded sucessfuly %s.", name.c_str());

    AssetsManager::RemoveTask(path);
}

void ShaderTask::doInBackground(std::string path)
{
    auto shader_map = ReadShaderOfType(path);

    Event::Add(&ShaderTask::upload, this, path, shader_map[ShaderType_Fragment], shader_map[ShaderType_Vertex]);
}

UniformStackInfo ShaderTask::getGLSLStackInfo(const std::string &source) {
    std::stringstream iss(source);

    std::string uniform, object, name;

    if (source.find("uniform") != std::string::npos) {
        if (!(iss >> uniform >> object >> name)) {
            throw std::runtime_error("Fail to get stack info");
        }
    } else {
        if (!(iss >> object >> name)) {
            throw std::runtime_error("Fail to get stack info");
        }
    }

    auto pstart = name.find('[');
    auto pend = name.find(']');

    UniformStackInfo info;
    info.name = name.substr(0, pstart);

    info.object = object;
    info.count = std::atoi(name.substr(pstart + 1, name.length() - pend).c_str());

    return info;
}

std::string ShaderTask::getStructName(const std::string &source) {
    std::stringstream iss(source);

    std::string discart, name;

    if (!(iss >> discart >> name >> discart)) {
        throw std::runtime_error("Fail to inspect struct.");
    }

    return name;
}

void ShaderTask::insert_field(const std::string &line, const std::string &type_name, const std::string &name, std::vector<Uniform> &vector, std::map<std::string, std::vector<Uniform>> &struct_map) {
    UniformType type = UniformType_object;
    if (type_name == "float") {
        type = UniformType_float;
    } else if (type_name == "int") {
        type = UniformType_int;
    } else if (type_name == "bool") {
        type = UniformType_bool;
    } else if (type_name == "samplerCube") {
        type = UniformType_samplerCube;
    } else if (type_name == "sampler2D") {
        type = UniformType_sampler2D;
    } else if (type_name == "mat3") {
        type = UniformType_mat3;
    } else if (type_name == "mat4") {
        type = UniformType_mat4;
    } else if (type_name == "vec2") {
        type = UniformType_vec2;
    } else if (type_name == "vec3") {
        type = UniformType_vec3;
    } else if (type_name == "vec4") {
        type = UniformType_vec4;
    }

    if (line.find('[') != std::string::npos && line.find(']') != std::string::npos) {
        auto info = getGLSLStackInfo(line);
        vector.push_back({info.name, type_name, type});

        for (int i=0;i<info.count;i++) {
            if (type == UniformType_object) {
                for (auto un : struct_map[type_name]) {
                    vector.push_back({info.name + "[" + std::to_string(i) + "]." + un.name, un.type_name, un.type});
                }
                continue;
            }
            vector.push_back({info.name + "[" + std::to_string(i) + "]", type_name, type});
        }
    } else {
        vector.push_back({name, type_name, type});
    }
}

std::map<ShaderType, ShaderData> ShaderTask::ReadShaderOfType(const std::string &path) {
    std::ifstream is(path);
    if (!is.is_open()) {
        throw std::runtime_error("file don't open.");
    }

    auto tella = is.tellg();

    std::map<std::string, std::vector<Uniform>> object_map;

    bool isInNextLine = false;
    std::string line, acumulate, acumulate_data;
    while (std::getline(is, line)) {
        std::stringstream iss(line);
        if (line.find("struct") != std::string::npos || isInNextLine) {
            if ((line.find('{') != std::string::npos)) {
                if (!isInNextLine) {
                    acumulate = line;
                    // continue;
                } else {
                    acumulate += line + ' ';

                }
                isInNextLine = false;
            } else {
                isInNextLine = true;
                acumulate += line + ' ';
                continue;
            }
            object_map.emplace(getStructName(acumulate), std::vector<Uniform>());
            continue;
        }

        if (!acumulate.empty()) {
            if (line.find('}') == std::string::npos) {
                std::string type_name, name;

                if (!(iss >> type_name >> name)) {
                    continue;
                }

                name.erase(std::remove(name.begin(), name.end(), ';'), name.end());

                insert_field(line, type_name, name, object_map[getStructName(acumulate)], object_map);
            } else {
                acumulate.clear();
            }
        }
    }

    is.close();

    is.open(path);

    std::map<ShaderType, ShaderData> shader_map;

    ShaderType type = ShaderType_None;
    
    while (std::getline(is, line)) {
        std::stringstream iss(line);
        if (line.find("shader_type") != std::string::npos) {
            if (line.find("vert") != std::string::npos) {
                type = ShaderType_Vertex;
                continue;
            } else if (line.find("frag") != std::string::npos) {
                type = ShaderType_Fragment;
                continue;
            }
        }

        auto &data = shader_map[type];
        data.source += line + '\n';

        if (line.find("uniform") != std::string::npos) {
            std::string uniform, type_name, name;

            if (!(iss >> uniform >> type_name >> name)) {
                continue;
            }
            name.erase(std::remove(name.begin(), name.end(), ';'), name.end());

            insert_field(line, type_name, name, data.uniformList, object_map);
        }
    }
    is.close();

    return shader_map;
}

grr::UniformType ShaderTask::ConvertToGType(UniformType type)
{
    switch (type)
    {
    case UniformType_int:
    case UniformType_bool:
    case UniformType_sampler2D:
    case UniformType_samplerCube:
        return grr::UniformType_int;
    case UniformType_float:
        return grr::UniformType_float;
    case UniformType_mat3:
        return grr::UniformType_mat3;
    case UniformType_mat4:
        return grr::UniformType_mat4;
    case UniformType_vec2:
        return grr::UniformType_vec2;
    case UniformType_vec3:
        return grr::UniformType_vec3;
    case UniformType_vec4:
        return grr::UniformType_vec4;
    default:
        return grr::UniformType_none;
    }
}

bool ShaderTask::buildShader(const std::string &name, grr::gShader *shader, ShaderData frag, ShaderData vert)
{
    auto fragment_source = frag.source.c_str();
    auto vertex_source = vert.source.c_str();

    shader->cleanUniform();

    shader->build(&fragment_source, 1, &vertex_source, 1);
    if (!shader->isValid())
    {
        Console::LogError("[ ShaderTask ] Shader.Build Fail to build shader, %s, %s", name.c_str(), grr::error());
        // AssetsManager::RemoveShader(shader);
        return false;
    }

    grr::UniformType type = grr::UniformType_none;
    for (auto uniform : frag.uniformList)
    {
        type = ConvertToGType(uniform.type);
        if (type == grr::UniformType_none) {
            Console::LogError("[ ShaderTask ] Shader.UniformType '%s %s', %s in fragment", uniform.type_name.c_str(), uniform.name.c_str(), name.c_str());
            continue;
        }

        PRINT_ERROR(shader->registry(uniform.name.c_str(), type));
    }

    for (auto uniform : vert.uniformList) {
        type = ConvertToGType(uniform.type);
        if (type == grr::UniformType_none)
        {
            Console::LogError("[ ShaderTask ] Shader.UniformType uniform '%s %s', %s in vertex", uniform.type_name.c_str(), uniform.name.c_str(), name.c_str());
            continue;
        }

        PRINT_ERROR(shader->registry(uniform.name.c_str(), type));
    }
    return true;
}




