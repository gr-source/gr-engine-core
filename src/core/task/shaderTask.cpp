#include "gr-base/core/task/shaderTask.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/event.hpp"
#include "imgui.h"

#include <gr-render/gShader.h>
#include <gr-render/gError.h>

#include <filesystem>
#include <fstream>

#define PRINT_ERROR(func) \
    if (func == -1) { \
        Console::LogError("[ ShaderTask ] Shader.Regiter uniform: %s", grr::error()); \
    }

std::unordered_map<std::string, grr::UniformType> ShaderTask::mUniformNameToType =
{
    {"float",       grr::UniformType_float},
    {"int",         grr::UniformType_int},
    {"bool",        grr::UniformType_bool},
    {"samplerCube", grr::UniformType_samplerCube},
    {"sampler2D",   grr::UniformType_sampler2D},
    {"mat3",        grr::UniformType_mat3},
    {"mat4",        grr::UniformType_mat4},
    {"vec2",        grr::UniformType_vec2},
    {"vec3",        grr::UniformType_vec3},
    {"vec4",        grr::UniformType_vec4},
};

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

void ShaderTask::upload(std::string path)
{
    auto name = std::filesystem::path(path).filename().string();
    if (!buildShader(name, m_shader)) {
        return AssetsManager::RemoveTask(path);
    }

    invoke(m_shader);

    Console::LogInfo("[ ShaderTask ] Shader loaded sucessfuly %s.", name.c_str());

    AssetsManager::RemoveTask(path);
}

void ShaderTask::doInBackground(std::string path)
{
    m_shader = m_shader != nullptr ? m_shader : AssetsManager::CreateShader(path);

    try {
        bindShader(path);
    } catch (const std::exception &e)
    {
        Console::LogError("[ ShaderTask ] Failt to loas shader: " + path);

        return AssetsManager::RemoveTask(path);
    }
 
    Event::Add(&ShaderTask::upload, this, path);
}

std::map<ShaderType, std::string> ShaderTask::LoadShaderBase(ShaderUser type)
{
    std::map<ShaderType, std::string> mShaderMap;

    auto path = File::GetAsset(type == ShaderUser_shader3D ? "/shader/model/shader3D.glsl" : "/shader/model/shader2D.glsl", true);

    std::ifstream is(path);
    if (!is.is_open())
    {
        throw std::runtime_error("Fail to load file: " + path);
    }

    ShaderType shaderType = ShaderType_None;

    std::string line;
    while (std::getline(is, line))
    {
        if (line.find("define_type") != std::string::npos)
        {
            if (line.find("frag") != std::string::npos)
            {
                shaderType = ShaderType_Fragment;
            }
            else if (line.find("vert") != std::string::npos)
            {
                shaderType = ShaderType_Vertex;
            }
            continue;
        }
        mShaderMap[shaderType] += line + '\n';
    }
    is.close();

    return mShaderMap;
}

bool ShaderTask::ValidStr(const std::string &str, std::string *data, int count)
{
    std::stringstream iss(str);

    while (iss >> data[(--count) < 0 ? 0 : count])
    {
        if (count < 0)
        {
            return false;
        }
    }
    return true;
}

std::vector<std::tuple<std::string, std::string>> ShaderTask::ReadStructField(const std::string &content)
{
    std::stringstream iss(content);
    
    std::vector<std::tuple<std::string, std::string>> fieldList;
    std::string line;
    while (std::getline(iss, line))
    {
        std::size_t lastPos = 0;
        while (true)
        {
            auto pos = line.find(';', lastPos);
            if (pos == std::string::npos)
            {
                break;
            }

            auto fieldline = line.substr(lastPos, pos - lastPos);
            if (fieldline.find('/') != std::string::npos)
                break;

            std::string params[2];
            if (!ValidStr(fieldline, params, 2))
            {
                continue;
            }

            auto &field = fieldList.emplace_back(params[0], params[1]);
            
            lastPos = pos + 1;
        }
    }
    return fieldList;
}

std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> ShaderTask::ReadStuct(const std::string &content)
{
    std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> structMap;
    std::size_t lastPos = 0;
    while (true)
    {
        auto pos = content.find("struct", lastPos);
        if (pos == std::string::npos)
        {
            break;
        }

        auto start_struct_pos = content.find('{', pos);

        std::string struct_name;

        lastPos = pos + 6;
        if (!ValidStr(content.substr(pos + 6, start_struct_pos - (pos + 6)), &struct_name, 1))
            continue;
        
        auto end_struct_pos = content.find('}', start_struct_pos);

        auto fieldList = ReadStructField(content.substr(start_struct_pos + 1, end_struct_pos - (start_struct_pos + 1)));
        if (fieldList.empty())
        {
            continue;
        }

        if (structMap.find(struct_name) != structMap.end())
        {
            continue;
        }

        structMap.emplace(struct_name, fieldList);
    }
    return structMap;
}

bool ShaderTask::IsArray(const std::string &content)
{
    return content.find('[') != std::string::npos;
}

std::size_t ShaderTask::GetArrayCount(const std::string &content, std::string &name)
{
    auto start_array = content.find('[');
    if (start_array == std::string::npos)
    {
        return 0;
    }
    
    auto end_array = content.find(']', start_array);
    if (end_array == std::string::npos)
    {
        return 0;
    }

    name = content.substr(0, start_array);

    return std::atoi(content.substr(start_array + 1, end_array - (start_array + 1)).c_str());
}

void ShaderTask::SetUniformList(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> &structMap, const std::string &name, const std::string &type, std::vector<std::tuple<std::string, std::string>> &vector, bool makeArray)
{
    if (IsArray(name) && !makeArray)
    {
        std::string array_name;

        auto count = GetArrayCount(name, array_name);

        bool isArrayGlsl = structMap.find(type) == structMap.end();

        for (std::size_t i={};i<count;i++)
        {
            auto array_item(array_name + '[' + std::to_string(i) + "]");
            if (isArrayGlsl)
            {
                vector.emplace_back(array_item, type);
                continue;
            }

            SetUniformList(structMap, array_item, type, vector, true);
        }
        return;
    }

    auto it = structMap.find(type);
    if (it == structMap.end())
    {
        vector.emplace_back(name, type);
        return;
    }

    for (auto &member : it->second)
    {
        auto &&[field_name, type] = member;

        SetUniformList(structMap, name + "." + field_name, type, vector, makeArray);
    }
}

std::vector<std::tuple<std::string, std::string>> ShaderTask::GetRegistryField(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> &structMap, std::unordered_map<std::string, std::string> &uniformMap)
{
    std::vector<std::tuple<std::string, std::string>> uniformFromRegister;
    for (auto &&[name, type] : uniformMap)
    {
        SetUniformList(structMap, name, type, uniformFromRegister);
    }

    return uniformFromRegister;
}

std::unordered_map<std::string, std::string> ShaderTask::ReadUniform(const std::string &content)
{
    std::unordered_map<std::string, std::string> uniformMap;

    std::size_t lastUniform = 0;
    while (true)
    {
        auto start_uniform = content.find("uniform", lastUniform);
        if (start_uniform == std::string::npos)
        {
            break;
        }
        lastUniform = start_uniform + 7;

        auto end_uniform = content.find(';', lastUniform);
        if (end_uniform == std::string::npos)
        {
            continue;
        }

        auto uniform_line = content.substr(lastUniform, end_uniform - lastUniform);

        std::string split[2];
        if (!ValidStr(uniform_line, split, 2))
        {
            continue;
        }
        
        if (uniformMap.find(split[0]) != uniformMap.end())
        {
            continue;
        }

        uniformMap.emplace(split[0], split[1]);
    }

    return uniformMap;
}

std::string ShaderTask::RemoveCommenter(const std::string &source)
{
    std::string result;
    std::istringstream iss(source);

    std::string line;
    while (std::getline(iss, line))
    {
        auto start_comment = line.find("//");
        if (start_comment != std::string::npos)
        {
            line = line.substr(0, start_comment);
            if (line.empty())
            {
                continue;
            }
        }

        result += line + '\n';
    }

    std::size_t lastBlockComment = 0;
    while (true)
    {
        auto start_block_comment = result.find("/*", lastBlockComment);
        if (start_block_comment == std::string::npos)
        {
            break;
        }

        auto end_block_comment = result.find("*/", start_block_comment);

        result.erase(start_block_comment, (end_block_comment + 2) - start_block_comment);

        lastBlockComment = start_block_comment + 2;
    }
    
    return result;
}

void ShaderTask::bindShader(const std::string &path)
{
    std::ifstream is(path);
    if (!is.is_open())
    {
        throw std::runtime_error("Fail to open file: " + path);
    }

    std::vector<std::string> defineMap;

    ShaderUser shaderUser = ShaderUser_none;

    ShaderType shaderType = ShaderType_None;
    std::map<ShaderType, std::string> mSourceMap;

    bool isInDefineType = false;

    std::string line;
    while (std::getline(is, line))
    {
        if (line.find("define_type") != std::string::npos)
        {
            if (line.find("shader2D") != std::string::npos)
            {
                shaderUser = ShaderUser_shader2D;
            } else if (line.find("shader3D") != std::string::npos)
            {
                shaderUser = ShaderUser_shader3D;
            }
            isInDefineType = true;
            continue;
        } else if (line.find("define_raster") != std::string::npos)
        {
            if (line.find("frag") != std::string::npos)
            {
                shaderType = ShaderType_Fragment;
            } else if (line.find("vert") != std::string::npos)
            {
                shaderType = ShaderType_Vertex;
            }
            isInDefineType = false;
            continue;
        }

        if (isInDefineType && (line.find("define") != std::string::npos || line.find("undef") != std::string::npos))
        {
            defineMap.push_back(line);
            continue;
        }

        if (shaderType == ShaderType_None)
        {
            continue;
        }

        mSourceMap[shaderType] += line + '\n';
    }

    is.close();

    if (shaderUser == ShaderUser_none)
    {
        throw std::runtime_error("Shader do not define, please user '#define shader2D' or '#define shader3D'");
    }

    // Create User property
    {
        auto property = AssetsManager::GetShaderProperty(m_shader);
        if (property == nullptr)
        {
            property = AssetsManager::CreateShaderProperty(m_shader);
        }
        grr::UniformType type = grr::UniformType_none;

        // fragment
        {
            for (auto &&[name, type_name] : ReadUniform(RemoveCommenter(mSourceMap[ShaderType_Fragment])))
            {
                auto it = mUniformNameToType.find(type_name);
                if (it != mUniformNameToType.end())
                {
                    type = it->second;
                }

                property->addProperty(type_name, name, type);
            }
        }

        // vertex
        {
            for (auto &&[name, type_name] : ReadUniform(RemoveCommenter(mSourceMap[ShaderType_Vertex])))
            {
                auto it = mUniformNameToType.find(type_name);
                if (it != mUniformNameToType.end())
                {
                    type = it->second;
                }

                property->addProperty(type_name, name, type);
            }
        }
        // if (mUniformNameToType.find)
        // property->addProperty()
    }

    {
        m_shaderSource[ShaderType_Fragment] += "#version 330 core\n";
        m_shaderSource[ShaderType_Vertex]   += "#version 330 core\n";

        auto baseShaderSource = LoadShaderBase(shaderUser);

        for (auto defination : defineMap)
        {
            m_shaderSource[ShaderType_Fragment] += defination + '\n';
            m_shaderSource[ShaderType_Vertex]   += defination + '\n';
        }

        m_shaderSource[ShaderType_Fragment] += baseShaderSource[ShaderType_Fragment];
        m_shaderSource[ShaderType_Vertex]   += baseShaderSource[ShaderType_Vertex];
    }

    m_shaderSource[ShaderType_Fragment] += mSourceMap[ShaderType_Fragment];
    m_shaderSource[ShaderType_Vertex]   += mSourceMap[ShaderType_Vertex];

    m_shaderSource[ShaderType_Fragment] = RemoveCommenter(m_shaderSource[ShaderType_Fragment]);
    m_shaderSource[ShaderType_Vertex] = RemoveCommenter(m_shaderSource[ShaderType_Vertex]);

    // std::cout << "===== Vertex =====" << std::endl;
    // std::cout << m_shaderSource[ShaderType_Vertex] << std::endl;

    // fragment
    {
        auto structList = ReadStuct(m_shaderSource[ShaderType_Fragment]);
        auto uniformMap = ReadUniform(m_shaderSource[ShaderType_Fragment]);

        m_uniformList.emplace(ShaderType_Fragment, GetRegistryField(structList, uniformMap));
    }

    // vertex
    {
        auto structList = ReadStuct(m_shaderSource[ShaderType_Vertex]);
        auto uniformMap = ReadUniform(m_shaderSource[ShaderType_Vertex]);

        m_uniformList.emplace(ShaderType_Vertex, GetRegistryField(structList, uniformMap));
    }
}

bool ShaderTask::buildShader(const std::string &name, grr::gShader *shader)
{
    auto fragment_source = m_shaderSource[ShaderType_Fragment].c_str();
    auto vertex_source = m_shaderSource[ShaderType_Vertex].c_str();

    shader->cleanUniform();

    shader->build(&fragment_source, 1, &vertex_source, 1);
    if (!shader->isValid())
    {
        Console::LogError("[ ShaderTask ] Shader.Build Fail to build shader, %s, %s", name.c_str(), grr::error());
        // AssetsManager::RemoveShader(shader);
        return false;
    }

    for (auto &&[name, type] : m_uniformList[ShaderType_Fragment])
    {
        auto it = mUniformNameToType.find(type);
        if (it == mUniformNameToType.end())
        {
            Console::LogWarning("[ ShaderTask ] Fragment Shader uniform type do not is valid: %s, %s", name.c_str(), type.c_str());
            continue;
        }

        PRINT_ERROR(shader->registry(name.c_str(), 1, it->second));
    }
    
    for (auto &&[name, type] : m_uniformList[ShaderType_Vertex]) {
        auto it = mUniformNameToType.find(type);
        if (it == mUniformNameToType.end())
        {
            Console::LogWarning("[ ShaderTask ] Vertex Shader uniform type do not is valid: %s, %s", name.c_str(), type.c_str());
            continue;
        }

        PRINT_ERROR(shader->registry(name.c_str(), 1, it->second));
    }
    return true;
}




