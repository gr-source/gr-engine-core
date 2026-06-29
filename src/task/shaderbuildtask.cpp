#include "gr-base/task/shaderbuildtask.hpp"

#include "gr-base/asset/shaderasset.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/assetDB.hpp"

#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/geometry/shaderData.hpp"

#include "gr-base/core/file/file.hpp"
#include "gr-base/core/thread/taskmanager.hpp"
#include "gr-base/task/shadersavetask.hpp"

#include <exception>
#include <stdexcept>
#include <cstring>
#include <fstream>
#include <array>

ShaderBuildTask::ShaderBuildTask(const std::string& file, const URI& uri) : m_file(file), m_uri(uri)
{}

void ShaderBuildTask::onPreExecute()
{
    auto name = m_uri.toString();

    Console::LogInfo("[ ShaderBuildTask ] Building shader file '%s'", name.c_str());
}

ShaderAsset ShaderBuildTask::doInBackground()
{
    return onStartParceShader();
}

void ShaderBuildTask::onPostExecute(Result<ShaderAsset> result)
{
    auto name = m_uri.toString();

    Console::LogInfo("[ ShaderBuildTask ] Compiling '%s'...", name.c_str());

    if (!result.success)
    {
        Console::LogError("[ ShaderBuildTask ] falied in to build shader '%s'", name.c_str());
        Console::LogError("%s", result.error.c_str());

        return;
    }

    if (!compile(&result.value))
        return;

    Console::LogInfo("[ShaderBuildTask] Shader building sucessfully '%s'", name.c_str());

    TaskManager::Add<ShaderSaveTask>(m_uri, result.value);
}

bool ShaderBuildTask::compile(const ShaderAsset* asset)
{
    std::string fragment = "#version 420\n";
    fragment += asset->fragment;

    std::string vertex = "#version 420\n";
    vertex += asset->vertex;

    const char* fragment_src = fragment.c_str();
    const char* vertex_src = vertex.c_str();

    std::string error;

    gr::Shader shader;
    if (!shader.create(&fragment_src, 1, &vertex_src, 1, &error))
    {
        Console::LogError("[ ShaderBuildTask ] %s", error.c_str());
        return false;
    }
    shader.destroy();

    return true;
}

ShaderAsset ShaderBuildTask::onStartParceShader()
{
    std::vector<ShaderUniformRaw> s_shader_uniform_list;
    std::array<std::string, 3> resultShaderSource;

    ShaderAsset asset;

    auto&& [fragment, vertex, definations, shaderType] = ReadingSourceFileShader();

    ShaderAssetFlags_ flags = ShaderAssetFlags_Shader3D;
    if (shaderType == ShaderType::LINE)
        flags |= ShaderAssetFlags_Line;

    asset.flags = flags;

    auto baseShaderSource = LoadShaderBase(shaderType);
    for (auto defination : definations)
    {
        resultShaderSource[static_cast<int>(static_cast<int>(ShaderBuildType::FRAGMENT))] += defination + '\n';
        resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)]   += defination + '\n';
    }

    resultShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)] += baseShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)];
    resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)]   += baseShaderSource[static_cast<int>(ShaderBuildType::VERTEX)];

    resultShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)] += fragment;
    resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)]   += vertex;

    resultShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)] = remove_comment(resultShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)]);
    resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)] = remove_comment(resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)]);

    std::unordered_set<std::string> macro_map;
    if (shaderType == ShaderType::SKY)
    {
        macro_map.emplace("ENABLE_SKYBOX");
    }

    /*
    else
    {
        // if (!(m_shaderParams.flags & ShaderOptimize_static))
        {
            if (false)
            {
                macro_map.emplace("USE_INSTANCED");
            } else
            {
                macro_map.emplace("USE_MODEL");
            }
            // if (m_shaderParams.flags & ShaderOptimize_animated)
            {
                // macro_map.emplace("ENABLE_ANIMATION");
            }
            // if (m_shaderParams.flags & ShaderOptimize_instanced)
            {
            }
        }
    }
    */


    // resultShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)] = process_macro(resultShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)], macro_map);
    // resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)] = process_macro(resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)], macro_map);

    // fragment
    {
        auto structList = ReadStuct(resultShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)]);
        auto uniformMap = ReadUniform(resultShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)]);

        auto vector = GetRegistryField(structList, uniformMap);
        if (!vector.empty())
        {
            s_shader_uniform_list.insert(s_shader_uniform_list.end(),
                    std::make_move_iterator(vector.begin()),
                    std::make_move_iterator(vector.end()));
        }
    }

    // vertex
    {
        auto structList = ReadStuct(resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)]);
        auto uniformMap = ReadUniform(resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)]);

        auto vector = GetRegistryField(structList, uniformMap);
        if (!vector.empty())
        {
            s_shader_uniform_list.insert(s_shader_uniform_list.end(),
                    std::make_move_iterator(vector.begin()),
                    std::make_move_iterator(vector.end()));
        }
    }

    auto name = m_uri.toString();

    asset.name = name;
    asset.fragment = resultShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)];
    asset.vertex = resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)];

    for (auto& uniform : s_shader_uniform_list)
        asset.AddUniform(uniform.name, uniform.variable, uniform.count);

    return asset;
}

std::tuple<std::string, std::string, std::vector<std::string>, ShaderType> ShaderBuildTask::ReadingSourceFileShader()
{
    std::ifstream is(m_file, std::ios_base::in);
    if (!is.is_open())
        throw std::runtime_error(strerror(errno));

    ShaderBuildType shaderBuildType = ShaderBuildType::NONE;
    ShaderType shaderType = ShaderType::NONE;

    bool bCanIsInDefined = false;

    std::vector<std::string> defineMap;

    std::array<std::string, IShaderStage> mCustomSourceShaderMap;

    std::string line;
    while (std::getline(is, line))
    {
        if (line.find("define_type") != std::string::npos)
        {
            if (line.find("shader2D") != std::string::npos)
                shaderType = ShaderType::SHADER2D;
            else if (line.find("shader3D") != std::string::npos)
                shaderType = ShaderType::SHADER3D;
            else if (line.find("sky") != std::string::npos)
                shaderType = ShaderType::SKY;
            else if (line.find("line") != std::string::npos)
                shaderType = ShaderType::LINE;

            bCanIsInDefined = true;

            continue;
        } else if (line.find("define_raster") != std::string::npos)
        {
            if (line.find("frag") != std::string::npos)
            {
                shaderBuildType = ShaderBuildType::FRAGMENT;
            } else if (line.find("vert") != std::string::npos)
            {
                shaderBuildType = ShaderBuildType::VERTEX;
            }

            bCanIsInDefined = false;
            continue;
        }

        if (bCanIsInDefined && (line.find("define") != std::string::npos || line.find("undef") != std::string::npos))
        {
            defineMap.push_back(line);
            continue;
        }

        if (shaderBuildType != ShaderBuildType::NONE)
            mCustomSourceShaderMap[static_cast<int>(shaderBuildType)] += line + '\n';

    }
    is.close();

    return std::make_tuple(mCustomSourceShaderMap[static_cast<int>(ShaderBuildType::FRAGMENT)], mCustomSourceShaderMap[static_cast<int>(ShaderBuildType::VERTEX)], defineMap, shaderType);
}


std::array<std::string, IShaderStage> ShaderBuildTask::LoadShaderBase(ShaderType type)
{
    std::array<std::string, IShaderStage> mShaderMap;

    std::string path_to_base;
    switch (type)
    {
        case ShaderType::SHADER2D:
            path_to_base = "/assets/shader/model/shader2D.glsl";
            break;
        case ShaderType::LINE:
            path_to_base = "/assets/shader/model/shader3D_line.glsl";
            break;
        case ShaderType::SHADER3D:
        case ShaderType::SKY:
            path_to_base = "/assets/shader/model/shader3D.glsl";
            break;
        default:
            break;
    }

    auto path = gfile::get_data_directory() + path_to_base;

    std::ifstream is(path);
    if (!is.is_open())
    {
        throw std::runtime_error("Fail to open file: " + path);
    }

    ShaderBuildType shaderBuildType = ShaderBuildType::FRAGMENT;

    std::string line;
    while (std::getline(is, line))
    {
        if (line.find("define_type") != std::string::npos)
        {
            if (line.find("frag") != std::string::npos)
            {
                shaderBuildType = ShaderBuildType::FRAGMENT;
            }
            else if (line.find("vert") != std::string::npos)
            {
                shaderBuildType = ShaderBuildType::VERTEX;
            }
            continue;
        }
        mShaderMap[static_cast<std::uint8_t>(shaderBuildType)] += line + '\n';
    }
    is.close();

    return mShaderMap;
}

std::string ShaderBuildTask::remove_comment(const std::string &str)
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

#include <sstream>
#include <stack>

std::string ShaderBuildTask::process_macro(const std::string &source, const std::unordered_set<std::string> &defines)
{
    std::istringstream input(source);
    std::ostringstream output;
    std::string line;

    std::stack<bool> stateStack;
    bool currentState = true;

    while (std::getline(input, line)) {
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t"));

        if (trimmed.rfind("#ifdef", 0) == 0) {
            std::string macro = trimmed.substr(6);
            macro.erase(0, macro.find_first_not_of(" \t"));
            bool result = defines.count(macro) > 0;
            stateStack.push(currentState);
            currentState = currentState && result;
        }
        else if (trimmed.rfind("#ifndef", 0) == 0) {
            std::string macro = trimmed.substr(7);
            macro.erase(0, macro.find_first_not_of(" \t"));
            bool result = defines.count(macro) == 0;
            stateStack.push(currentState);
            currentState = currentState && result;
        }
        else if (trimmed.rfind("#else", 0) == 0) {
            bool parentState = !stateStack.empty() ? stateStack.top() : true;
            currentState = parentState && !currentState;
        }
        else if (trimmed.rfind("#endif", 0) == 0) {
            if (!stateStack.empty()) {
                currentState = stateStack.top();
                stateStack.pop();
            }
        }
        else if (currentState) {
            output << line << "\n";
        }
    }

    return output.str();
}

std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> ShaderBuildTask::ReadStuct(const std::string &content)
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

bool ShaderBuildTask::ValidStr(const std::string &str, std::string *data, int count)
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

std::vector<std::tuple<std::string, std::string, bool>> ShaderBuildTask::ReadStructField(const std::string &content)
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

std::unordered_map<std::string, std::tuple<std::string, bool>> ShaderBuildTask::ReadUniform(const std::string &content)
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

std::vector<ShaderBuildTask::ShaderUniformRaw> ShaderBuildTask::GetRegistryField(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> &structMap, std::unordered_map<std::string, std::tuple<std::string, bool>> &uniformMap)
{
    std::vector<ShaderUniformRaw> uniformFromRegister;
    for (auto &&[name, tuple] : uniformMap)
    {
        auto &&[variablename, isarray] = tuple;

        SetUniformList(structMap, name, variablename, uniformFromRegister, isarray);
    }

    return uniformFromRegister;
}

void ShaderBuildTask::SetUniformList(std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> &structMap, const std::string &name, const std::string &type, std::vector<ShaderUniformRaw> &vector, bool isarray)
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

                    vector.emplace_back(ShaderUniformRaw{item_name, variablename, 1});
                }
            }
            return;
        }


        vector.emplace_back(ShaderUniformRaw{field_name, type, static_cast<u32>(count)});
        
        return;
    }

    auto it = structMap.find(type);
    if (it == structMap.end())
    {
        vector.emplace_back(ShaderUniformRaw{name, type, 1});
        return;
    }

    for (auto &member : it->second)
    {
        auto &&[field_name, type, iaarray] = member;

        SetUniformList(structMap, name + "." + field_name, type, vector, isarray);
    }
}

std::size_t ShaderBuildTask::GetArrayCount(const std::string &content, std::string &name)
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



