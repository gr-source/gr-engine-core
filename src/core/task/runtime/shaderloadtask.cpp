#include "gr-base/core/task/runtime/shaderloadtask.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/core/thread/asynctask.hpp"

#include <gr-render/gError.h>

#include <stdexcept>
#include <fstream>
#include <stack>

#define PRINT_ERROR(func, property, type) \
    if (func == GR_INVALID_ID) { \
        Console::LogWarning("[ ShaderLoadTask ] Shader.Regiter uniform: %s", grr::error()); \
    } else if (userfield.find(name) != userfield.end()) { \
        property->addProperty(variablename, name, type); \
    }

std::unordered_map<std::string, UniformType> ShaderLoadTask::mVariableNameToID =
{
    {"float",       UniformType::FLOAT},
    {"int",         UniformType::INT},
    {"bool",        UniformType::BOOL},
    {"samplerCube", UniformType::SAMPLERCUBE},
    {"sampler2D",   UniformType::SAMPLER2D},
    {"mat3",        UniformType::MAT3},
    {"mat4",        UniformType::MAT4},
    {"vec2",        UniformType::VEC2},
    {"vec3",        UniformType::VEC3},
    {"vec4",        UniformType::VEC4},
};

// public
ShaderLoadTask::ShaderLoadTask(ShaderParams shaderParams, grr::gShader *shader) : m_shaderParams(shaderParams), m_path(shaderParams.base), m_shader(shader), bCanCreateShader(shader == nullptr), IAsyncTask(TaskPriority::HIGH)
{}

void ShaderLoadTask::addFunction(std::function<void (grr::gShader *)> func)
{
    if (func == nullptr)
        return;

    std::lock_guard<std::mutex> lock(mutex);
    m_func_list.push_back(func);
}

// private
grr::gShader* ShaderLoadTask::doInBackground()
{
    if (!gfile::exists(m_path))
    {
        Console::LogError("[ ShaderLoadTask ] file %s, do not found", m_path.c_str());
        return m_shader;
    }

    m_shaderParams.type = get_shader_asset_type(m_path);

    m_shader = bCanCreateShader ? AssetsManager::CreateShader(m_shaderParams) : m_shader;

    try
    {
        bind_shader(m_path);
    } catch (const std::exception &e)
    {
        Console::LogError("[ ShaderLoadTask ] Failt to load shader %s: %s", m_path.c_str(), e.what());
        if (bCanCreateShader)
        {
            AssetsManager::RemoveShader(m_shader);
            m_shader = nullptr;
        }

        AssetsManager::RemoveTask(m_path);
        return m_shader;
    }

    return m_shader;
}

#include <filesystem>
void ShaderLoadTask::onPostExecute(grr::gShader* result)
{
    if (result == nullptr)
        return AssetsManager::RemoveTask(m_path);

    auto name = std::filesystem::path(m_path).filename().string();
    if (!buildShader(name, m_shader))
    {
        if (bCanCreateShader)
        {
            AssetsManager::RemoveShader(m_shader);
            m_shader = nullptr;
        }

        Console::LogError("[ ShaderLoadTask ] Fail to build shader: %s", m_path.c_str());

        return AssetsManager::RemoveTask(m_path);
    }

    Console::LogInfo("[ ShaderLoadTask ] Shader loaded sucessfully %s.", name.c_str());


    std::vector<std::function<void (grr::gShader *)>> funclist;
    {
        std::lock_guard<std::mutex> lock(mutex);
        funclist = std::move(m_func_list);
    }

    for (auto &func : funclist)
        func(m_shader);

    AssetsManager::RemoveTask(m_path);
}

ShaderType ShaderLoadTask::get_shader_asset_type(const std::string &path)
{
    std::ifstream is(path);
    if (!is.is_open())
    {
        throw std::runtime_error("File do not open.");
    }

    ShaderType type = ShaderType::SHADER2D;

    std::string line;
    while (std::getline(is, line))
    {
        if (line.find("#define_type") == std::string::npos)
        {
            continue;
        }

        if (line.find("shader2D") != std::string::npos)
            type = ShaderType::SHADER2D;
        else if (line.find("shader3D") != std::string::npos)
            type = ShaderType::SHADER3D;
        else if (line.find("sky") != std::string::npos)
            type = ShaderType::SKY;
        else if (line.find("line") != std::string::npos)
            type = ShaderType::LINE;

        break;
    }

    is.close();

    return type;
}

std::array<std::string, IShaderStage> ShaderLoadTask::LoadShaderBase(ShaderType type)
{
    std::array<std::string, IShaderStage> mShaderMap;

    std::string path_to_base;
    switch (type)
    {
        case ShaderType::SHADER2D:
            path_to_base = "/assets/shader/model/shader2D.glsl";
            break;
        case ShaderType::SHADER3D:
        case ShaderType::LINE:
        case ShaderType::SKY:
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


        vector.emplace_back(ShaderUniform{field_name, type, static_cast<u32>(count)});
        
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

    ShaderStage_ shaderstage = ShaderStage::FRAGMENT;
    std::array<std::string, IShaderStage> mCustomSourceShaderMap;

    bool isInDefineType = false;

    std::string line;
    while (std::getline(is, line))
    {
        if (line.find("define_type") != std::string::npos)
        {
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

        mCustomSourceShaderMap[shaderstage] += line + '\n';
    }

    is.close();

    // Create User property
    {
        UniformType variable = UniformType::BOOL;

        // fragment
        {
            for (auto &&[name, tuple] : ReadUniform(remove_comment(mCustomSourceShaderMap[ShaderStage::FRAGMENT])))
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
            for (auto &&[name, tuple] : ReadUniform(remove_comment(mCustomSourceShaderMap[ShaderStage::VERTEX])))
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

        auto baseShaderSource = LoadShaderBase(m_shaderParams.type);
        for (auto defination : defineMap)
        {
            m_shaderSource[ShaderStage::FRAGMENT] += defination + '\n';
            m_shaderSource[ShaderStage::VERTEX]   += defination + '\n';
        }

        m_shaderSource[ShaderStage::FRAGMENT] += baseShaderSource[ShaderStage::FRAGMENT];
        m_shaderSource[ShaderStage::VERTEX]   += baseShaderSource[ShaderStage::VERTEX];
    }

    m_shaderSource[ShaderStage::FRAGMENT] += mCustomSourceShaderMap[ShaderStage::FRAGMENT];
    m_shaderSource[ShaderStage::VERTEX]   += mCustomSourceShaderMap[ShaderStage::VERTEX];

    m_shaderSource[ShaderStage::FRAGMENT] = remove_comment(m_shaderSource[ShaderStage::FRAGMENT]);
    m_shaderSource[ShaderStage::VERTEX] = remove_comment(m_shaderSource[ShaderStage::VERTEX]);

    std::unordered_set<std::string> macro_map;
    if (m_shaderParams.type == ShaderType::LINE)
    {
        macro_map.emplace("ENABLE_LINE");
    } else if (m_shaderParams.type == ShaderType::SKY)
    {
        macro_map.emplace("ENABLE_SKYBOX");
    }
    else
    {
        if (!(m_shaderParams.flags & ShaderOptimize_static))
        {
            if (m_shaderParams.flags & ShaderOptimize_dynamic)
            {
                macro_map.emplace("USE_MODEL");
            }
            if (m_shaderParams.flags & ShaderOptimize_animated)
            {
                macro_map.emplace("ENABLE_ANIMATION");
            }
            if (m_shaderParams.flags & ShaderOptimize_instanced)
            {
                macro_map.emplace("USE_INSTANCED");
            }
        }
    }

    m_shaderSource[ShaderStage::FRAGMENT] = process_macro(m_shaderSource[ShaderStage::FRAGMENT], macro_map);
    m_shaderSource[ShaderStage::VERTEX] = process_macro(m_shaderSource[ShaderStage::VERTEX], macro_map);

    // fragment
    {
        auto structList = ReadStuct(m_shaderSource[ShaderStage::FRAGMENT]);
        auto uniformMap = ReadUniform(m_shaderSource[ShaderStage::FRAGMENT]);

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
        auto structList = ReadStuct(m_shaderSource[ShaderStage::VERTEX]);
        auto uniformMap = ReadUniform(m_shaderSource[ShaderStage::VERTEX]);

        auto vector = GetRegistryField(structList, uniformMap);
        if (!vector.empty())
        {
            s_shader_uniform_list.insert(s_shader_uniform_list.end(),
                    std::make_move_iterator(vector.begin()),
                    std::make_move_iterator(vector.end()));
        }
    }
}

bool ShaderLoadTask::buildShader(const std::string &name, grr::gShader *shader)
{
    auto fragment_source = m_shaderSource[ShaderStage::FRAGMENT].c_str();
    auto vertex_source = m_shaderSource[ShaderStage::VERTEX].c_str();

    shader->build(&fragment_source, 1, &vertex_source, 1);
    if (!shader->isValid())
    {
        Console::LogError("[ ShaderLoadTask ] Shader.Build Fail to build shader, %s, %s", name.c_str(), grr::error());
        return false;
    }

    auto property = AssetsManager::GetShaderProperty(m_shader);
    if (property == nullptr)
        property = AssetsManager::CreateShaderProperty(m_shader);
    property->clear();

    for (auto &&[name, variablename, count] : s_shader_uniform_list)
    {
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

std::string ShaderLoadTask::process_macro(const std::string &source, const std::unordered_set<std::string> &defines)
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

