#include "gr-base/core/task/shaderloadtask.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/core/task/shaderProperty.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/event.hpp"

#include "gr-base/core/file/file.hpp"

#include <gr-render/gShader.h>
#include <gr-render/gError.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <tuple>

#define PRINT_ERROR(func, property, type) \
    if (func == InvalidUniformID) { \
        Console::LogError("[ ShaderLoadTask ] Shader.Regiter uniform: %s", grr::error()); \
    } else if (userfield.find(name) != userfield.end()) { \
        property->addProperty(variablename, name, type); \
    }

std::unordered_map<std::string, UniformVariable_> ShaderLoadTask::mVariableNameToID =
{
    {"float",       UniformVariable::FLOAT},
    {"int",         UniformVariable::INT},
    {"bool",        UniformVariable::BOOL},
    {"samplerCube", UniformVariable::SAMPLERCUBE},
    {"sampler2D",   UniformVariable::SAMPLER2D},
    {"mat3",        UniformVariable::MAT3},
    {"mat4",        UniformVariable::MAT4},
    {"vec2",        UniformVariable::VEC2},
    {"vec3",        UniformVariable::VEC3},
    {"vec4",        UniformVariable::VEC4},
};

ShaderLoadTask::ShaderLoadTask(ShaderOptimize_ optimize, const std::string &path, ShaderFunc func, grr::gShader *shader) : m_shader(shader), bShaderCreated(shader == nullptr), task_name(path), shaderoptimize(optimize)
{
    if (func != nullptr)
        m_functions.push_back(func);

    Threadpool::AddTask(&ShaderLoadTask::doInBackground, this, path);
}

void ShaderLoadTask::addFunction(ShaderFunc func)
{
    m_functions.push_back(func);
}

void ShaderLoadTask::invoke(grr::gShader *shader)
{
    for (auto func : m_functions)
        func(shader);
}

void ShaderLoadTask::upload(std::string path)
{
    auto name = std::filesystem::path(path).filename().string();
    if (!buildShader(name, m_shader))
    {
        if (bShaderCreated)
        {
            AssetsManager::RemoveShader(m_shader);
        }

        Console::LogError("[ ShaderLoadTask ] Fail to build shader: %s", path.c_str());

        return AssetsManager::RemoveTask(task_name);
    }

    Console::LogInfo("[ ShaderLoadTask ] Shader loaded sucessfully %s.", name.c_str());
    
    invoke(m_shader);

    AssetsManager::RemoveTask(task_name);
}

void ShaderLoadTask::doInBackground(std::string path)
{
    if (!gfile::exists(path))
    {
        Console::LogWarning("[ ShaderLoadTask ] File not found: %s", path.c_str());

        return AssetsManager::RemoveTask(task_name);
    }

    m_asset_shader_type = get_shader_asset_type(path);

    m_shader = bShaderCreated ? AssetsManager::CreateShader(m_asset_shader_type, shaderoptimize, task_name) : m_shader;

    try {
        bind_shader(path);
    } catch (const std::exception &e)
    {
        Console::LogError("[ ShaderLoadTask ] Failt to load shader %s: %s", path.c_str(), e.what());
        if (bShaderCreated)
        {
            AssetsManager::RemoveShader(m_shader);
        }

        return AssetsManager::RemoveTask(task_name);
    }
 
    Event::Add(&ShaderLoadTask::upload, this, path);
}

AssetShader ShaderLoadTask::get_shader_asset_type(const std::string &name)
{
    std::ifstream is(name);
    if (!is.is_open())
    {
        throw std::runtime_error("File do not open.");
    }

    AssetShader type = AssetShader_None;

    std::string line;
    while (std::getline(is, line))
    {
        if (line.find("#define_type") == std::string::npos)
        {
            continue;
        }

        if (line.find("shader2D") != std::string::npos)
        {
            type = AssetShader_Shader2D;
        } else if (line.find("shader3D") != std::string::npos)
        {
            type = AssetShader_Shader3D;
        }

        break;
    }

    is.close();

    return type;
}

std::array<std::string, IShaderStage> ShaderLoadTask::LoadShaderBase(ShaderType_ type)
{
    std::array<std::string, IShaderStage> mShaderMap;

    std::string path_to_base;
    switch (type)
    {
        case ShaderType::SHADER2D:
            path_to_base = "/assets/shader/model/shader2D.glsl";
            break;
        case ShaderType::SHADER3D:
            path_to_base = "/assets/shader/model/shader3D.glsl";
            break;
        default:
            break;
    }

    auto path = gfile::get_data_directory(path_to_base);

    std::ifstream is(path);
    if (!is.is_open())
    {
        throw std::runtime_error("Fail to open file: " + path);
    }

    ShaderStage_ shaderstage = ShaderStage::FRAGMENT;

    std::string line;
    while (std::getline(is, line))
    {
        if (line.find("define_type") != std::string::npos)
        {
            if (line.find("frag") != std::string::npos)
            {
                shaderstage = ShaderStage::FRAGMENT;
            }
            else if (line.find("vert") != std::string::npos)
            {
                shaderstage = ShaderStage::VERTEX;
            }
            continue;
        }
        mShaderMap[static_cast<std::uint8_t>(shaderstage)] += line + '\n';
    }
    is.close();

    return mShaderMap;
}

bool ShaderLoadTask::ValidStr(const std::string &str, std::string *data, int count)
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

std::vector<std::tuple<std::string, std::string, bool>> ShaderLoadTask::ReadStructField(const std::string &content)
{
    std::stringstream iss(content);
    
    std::vector<std::tuple<std::string, std::string, bool>> fieldList;
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

            lastPos = pos + 1;

            std::string params[2];
            if (!ValidStr(fieldline, params, 2))
            {
                continue;
            }

            auto &&[name, variable] = params;

            fieldList.emplace_back(name, variable, name.find('[') != std::string::npos && name.find(']') != std::string::npos);
        }
    }
    return fieldList;
}

std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> ShaderLoadTask::ReadStuct(const std::string &content)
{
    std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> structMap;
    std::size_t end = 0, start = 0;

    while ((start = content.find("struct", end)) != std::string::npos)
    {
        auto start_struct_pos = content.find('{', start);

        std::string struct_name;

        end = start + 6;
        if (!ValidStr(content.substr(start + 6, start_struct_pos - (start + 6)), &struct_name, 1))
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

bool ShaderLoadTask::IsArray(const std::string &content)
{
    return content.find('[') != std::string::npos;
}

std::size_t ShaderLoadTask::GetArrayCount(const std::string &content, std::string &name)
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

void ShaderLoadTask::SetUniformList(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> &structMap, const std::string &name, const std::string &type, std::vector<ShaderUniform> &vector, bool isarray)
{
    if (isarray)
    {
        std::string field_name;

        auto count = GetArrayCount(name, field_name);

        bool bUserStruct = structMap.find(type) != structMap.end();
        if (bUserStruct)
        {
            for (auto &&[name, variablename, isarray] : structMap[type])
            {
                if (isarray)
                {
                    continue;
                }

                for (std::size_t i={};i<count;i++)
                {
                    auto item_name(field_name + '[' + std::to_string(i) + "]." + name);

                    vector.emplace_back(ShaderUniform{item_name, variablename, 1});
                }
            }
            return;
        }


        vector.emplace_back(ShaderUniform{field_name, type, static_cast<grm::u32>(count)});
        
        return;
    }

    auto it = structMap.find(type);
    if (it == structMap.end())
    {
        vector.emplace_back(ShaderUniform{name, type, 1});
        return;
    }

    for (auto &member : it->second)
    {
        auto &&[field_name, type, iaarray] = member;

        SetUniformList(structMap, name + "." + field_name, type, vector, isarray);
    }
}

std::vector<ShaderUniform> ShaderLoadTask::GetRegistryField(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> &structMap, std::unordered_map<std::string, std::tuple<std::string, bool>> &uniformMap)
{
    std::vector<ShaderUniform> uniformFromRegister;
    for (auto &&[name, tuple] : uniformMap)
    {
        auto &&[variablename, isarray] = tuple;

        SetUniformList(structMap, name, variablename, uniformFromRegister, isarray);
    }

    return uniformFromRegister;
}

std::unordered_map<std::string, std::tuple<std::string, bool>> ShaderLoadTask::ReadUniform(const std::string &content)
{
    std::unordered_map<std::string, std::tuple<std::string, bool>> uniformMap;

    std::size_t end = 0, start = 0;
    while ((start = content.find("uniform", end)) != std::string::npos)
    {
        end = start + 7;

        auto end_uniform = content.find(';', end);
        if (end_uniform == std::string::npos)
        {
            continue;
        }

        auto uniform_line = content.substr(end, end_uniform - end);

        std::string split[2];
        if (!ValidStr(uniform_line, split, 2))
        {
            continue;
        }

        auto &&[name, variable] = split;

        if (uniformMap.find(name) != uniformMap.end())
        {
            continue;
        }

        uniformMap.emplace(name, std::make_tuple(variable, name.find('[') != std::string::npos && name.find(']') != std::string::npos));
    }

    return uniformMap;
}

void ShaderLoadTask::bind_shader(const std::string &path)
{
    std::ifstream is(path);
    if (!is.is_open())
    {
        throw std::runtime_error("Fail to open file: " + path);
    }

    std::vector<std::string> defineMap;

    ShaderType_ shadertype = ShaderType::SHADER2D;

    ShaderStage_ shaderstage = ShaderStage::FRAGMENT;
    std::array<std::string, IShaderStage> mSourceMap;

    bool isInDefineType = false;

    std::string line;
    while (std::getline(is, line))
    {
        if (line.find("define_type") != std::string::npos)
        {
            if (line.find("shader2D") != std::string::npos)
            {
                shadertype = ShaderType::SHADER2D;
            } else if (line.find("shader3D") != std::string::npos)
            {
                shadertype = ShaderType::SHADER3D;
            }
            isInDefineType = true;
            continue;
        } else if (line.find("define_raster") != std::string::npos)
        {
            if (line.find("frag") != std::string::npos)
            {
                shaderstage = ShaderStage::FRAGMENT;
            } else if (line.find("vert") != std::string::npos)
            {
                shaderstage = ShaderStage::VERTEX;
            }
            isInDefineType = false;
            continue;
        }

        if (isInDefineType && (line.find("define") != std::string::npos || line.find("undef") != std::string::npos))
        {
            defineMap.push_back(line);
            continue;
        }


        mSourceMap[shaderstage] += line + '\n';
    }

    is.close();

    // Create User property
    {
        UniformVariable_ variable = UniformVariable::NONE;

        // fragment
        {
            for (auto &&[name, tuple] : ReadUniform(remove_comment(mSourceMap[ShaderStage::FRAGMENT])))
            {
                auto &&[variablename, isarray] = tuple;

                auto it = mVariableNameToID.find(variablename);
                if (it != mVariableNameToID.end())
                {
                    variable = it->second;
                }

                userfield.emplace(name);
            }
        }

        // vertex
        {
            for (auto &&[name, tuple] : ReadUniform(remove_comment(mSourceMap[ShaderStage::VERTEX])))
            {
                auto &&[variablename, isarray] = tuple;

                auto it = mVariableNameToID.find(variablename);
                if (it != mVariableNameToID.end())
                {
                    variable = it->second;
                }

                userfield.emplace(name);
            }
        }
    }

    {
        m_shaderSource[ShaderStage::FRAGMENT] += "#version 330 core\n";
        m_shaderSource[ShaderStage::VERTEX]   += "#version 330 core\n";

        auto baseShaderSource = LoadShaderBase(shadertype);

        for (auto defination : defineMap)
        {
            m_shaderSource[ShaderStage::FRAGMENT] += defination + '\n';
            m_shaderSource[ShaderStage::VERTEX]   += defination + '\n';
        }

        m_shaderSource[ShaderStage::FRAGMENT] += baseShaderSource[ShaderStage::FRAGMENT];
        m_shaderSource[ShaderStage::VERTEX]   += baseShaderSource[ShaderStage::VERTEX];
    }

    m_shaderSource[ShaderStage::FRAGMENT] += mSourceMap[ShaderStage::FRAGMENT];
    m_shaderSource[ShaderStage::VERTEX]   += mSourceMap[ShaderStage::VERTEX];

    m_shaderSource[ShaderStage::FRAGMENT] = remove_comment(m_shaderSource[ShaderStage::FRAGMENT]);
    m_shaderSource[ShaderStage::VERTEX] = remove_comment(m_shaderSource[ShaderStage::VERTEX]);

    std::unordered_set<std::string> macro_map;
    switch (shaderoptimize)
    {
        case ShaderOptimize_DYNAMIC:
            macro_map.emplace("USE_MODEL");
            break;
        case ShaderOptimize_ANIMATION_DYNAMIC:
            macro_map.emplace("ENABLE_ANIMATION");
            macro_map.emplace("USE_MODEL");
            break;
        default:
            break;
    }

    m_shaderSource[ShaderStage::FRAGMENT] = process_macro(m_shaderSource[ShaderStage::FRAGMENT], macro_map);
    m_shaderSource[ShaderStage::VERTEX] = process_macro(m_shaderSource[ShaderStage::VERTEX], macro_map);

    // fragment
    {
        auto structList = ReadStuct(m_shaderSource[ShaderStage::FRAGMENT]);
        auto uniformMap = ReadUniform(m_shaderSource[ShaderStage::FRAGMENT]);

        mShaderUniformMapToList[ShaderStage::FRAGMENT] = GetRegistryField(structList, uniformMap);
    }

    // vertex
    {
        auto structList = ReadStuct(m_shaderSource[ShaderStage::VERTEX]);
        auto uniformMap = ReadUniform(m_shaderSource[ShaderStage::VERTEX]);

        mShaderUniformMapToList[ShaderStage::VERTEX] = GetRegistryField(structList, uniformMap);
    }
}

bool ShaderLoadTask::buildShader(const std::string &name, grr::gShader *shader)
{
    auto fragment_source = m_shaderSource[ShaderStage::FRAGMENT].c_str();
    auto vertex_source = m_shaderSource[ShaderStage::VERTEX].c_str();

    shader->cleanUniform();

    shader->build(&fragment_source, 1, &vertex_source, 1);
    if (!shader->isValid())
    {
        Console::LogError("[ ShaderLoadTask ] Shader.Build Fail to build shader, %s, %s", name.c_str(), grr::error());
        return false;
    }

    auto property = AssetsManager::GetShaderProperty(m_shader);
    if (property == nullptr)
    {
        property = AssetsManager::CreateShaderProperty(m_shader);
    }
    property->clear();

    for (auto &&[name, variablename, count] : mShaderUniformMapToList[ShaderStage::FRAGMENT])
    {
        auto it = mVariableNameToID.find(variablename);
        if (it == mVariableNameToID.end())
        {
            Console::LogWarning("[ ShaderLoadTask ] Fragment Shader uniform type do not is valid: %s, %s", name.c_str(), variablename.c_str());
            continue;
        }

        PRINT_ERROR(shader->registry(name.c_str(), count, it->second), property, it->second);
    }
    
    for (auto &&[name, variablename, count] : mShaderUniformMapToList[ShaderStage::VERTEX]) {
        auto it = mVariableNameToID.find(variablename);
        if (it == mVariableNameToID.end())
        {
            Console::LogWarning("[ ShaderLoadTask ] Vertex Shader uniform type do not is valid: %s, %s", name.c_str(), variablename.c_str());
            continue;
        }

        PRINT_ERROR(shader->registry(name.c_str(), count, it->second), property, it->second);
    }
    return true;
}

std::string ShaderLoadTask::process_macro(const std::string &content, const std::unordered_set<std::string> &macro_map)
{
    std::istringstream iss(content);
    bool bcan_read_line = true;

    std::string output;

    std::string line;
    while (std::getline(iss, line))
    {
        if (line.find("#") != std::string::npos)
        {
            std::istringstream issk(line);
            if (line.find("ifdef") != std::string::npos)
            {
                std::string er, key;

                issk >> er >> key;

                bcan_read_line = false;

                if (macro_map.find(key) != macro_map.end())
                {
                    bcan_read_line = true;
                }

                continue;
            } else if (line.find("else") != std::string::npos)
            {
                bcan_read_line = !bcan_read_line;

                continue;
            } else if (line.find("endif") != std::string::npos)
            {
                bcan_read_line = true;

                continue;
            }
        }

        if (bcan_read_line)
            output += line + '\n';
    }
    return output;
}

std::string ShaderLoadTask::remove_comment(const std::string &str)
{
    std::string result;
    bool is_in_block = false;

    auto len = str.length();

    for (std::size_t i=0;i<len;i++)
    {
        if (is_in_block)
        {
            if ((i + 1 < len) && str[i] == '*' && str[i + 1] == '/')
            {
                is_in_block = false;

                i += 2;
            }
        } else
        {
            if ((i + 1 < len) && str[i] == '/' && str[i + 1] == '*')
            {
                is_in_block = true;

                i += 2;
            }

            if ((i + 1 < len) && str[i] == '/' && str[i + 1] == '/')
            {
                while (i < len && str[i] != '\n')
                    i++;
            }
        }

        if (!is_in_block)
        {
            result += str[i];
        }
    }
    return result;
}



