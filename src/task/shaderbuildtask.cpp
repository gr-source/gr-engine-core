#include "gr-base/task/shaderbuildtask.hpp"

#include "gr-base/asset/shaderasset.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/assetDB.hpp"

#include "gr-base/core/geometry/shaderData.hpp"

#include "gr-base/file/file.hpp"

#include "gr-base/task/assetSaveTask.hpp"

#include "gr-base/managers/taskmanager.hpp"

#include <cstring>
#include <fstream>
#include <cerrno>
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

    AssetID assetID = AssetDB::FindByHash(engine::hash::fnv1_str(name));
    if (assetID == INVALID_ASSET)
        assetID = AssetDB::CreateID();

    AssetSaveInfo info;
    info.uri = m_uri;
    info.id = assetID;
    info.type = AssetType_Shader;
    info.asset = std::make_shared<ShaderAsset>(result.value);

    TaskManager::Add<AssetSaveTask>(info);
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

    gr::shader shader;
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
    std::array<std::string, 3> resultShaderSource;

    auto&& [fragment, vertex, definations, domain] = ReadingSourceFileShader();

    auto baseShaderSource = LoadShaderBase(domain);
    for (auto defination : definations)
    {
        resultShaderSource[static_cast<int>(static_cast<int>(ShaderBuildType::FRAGMENT))] += defination + '\n';
        resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)]   += defination + '\n';
    }

    resultShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)] += baseShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)];
    resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)]   += baseShaderSource[static_cast<int>(ShaderBuildType::VERTEX)];

    resultShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)] += fragment;
    resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)]   += vertex;

    auto name = m_uri.toString();

    ShaderAsset asset;
    asset.domain = domain;
    asset.name = name;
    asset.fragment = resultShaderSource[static_cast<int>(ShaderBuildType::FRAGMENT)];
    asset.vertex = resultShaderSource[static_cast<int>(ShaderBuildType::VERTEX)];

    return asset;
}

std::tuple<std::string, std::string, std::vector<std::string>, ShaderDomain> ShaderBuildTask::ReadingSourceFileShader()
{
    std::ifstream is(m_file, std::ios_base::in);
    if (!is.is_open())
        throw std::runtime_error(strerror(errno));

    ShaderBuildType shaderBuildType = ShaderBuildType::NONE;
    ShaderDomain domain;

    bool bCanIsInDefined = false;

    std::vector<std::string> defineMap;

    std::array<std::string, IShaderStage> mCustomSourceShaderMap;

    std::string line;
    while (std::getline(is, line))
    {
        if (line.find("define_type") != std::string::npos)
        {
            if (line.find("sprite") != std::string::npos)
                domain = ShaderDomain::Sprite;
            else if (line.find("surface") != std::string::npos)
                domain = ShaderDomain::Surface;
            else if (line.find("debug") != std::string::npos)
                domain = ShaderDomain::Debug;
            else if (line.find("postprocess") != std::string::npos)
                domain = ShaderDomain::PostProcess;
            else
                throw std::runtime_error("shader domain not valid");

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

    return std::make_tuple(mCustomSourceShaderMap[static_cast<int>(ShaderBuildType::FRAGMENT)], mCustomSourceShaderMap[static_cast<int>(ShaderBuildType::VERTEX)], defineMap, domain);
}


std::array<std::string, IShaderStage> ShaderBuildTask::LoadShaderBase(ShaderDomain domain)
{
    std::array<std::string, IShaderStage> mShaderMap;

    std::string path_to_base;
    switch (domain)
    {
        case ShaderDomain::PostProcess:
            path_to_base = "/assets/shader/model/postprocess.glsl";
            break;
        case ShaderDomain::Sprite:
            path_to_base = "/assets/shader/model/shader2D.glsl";
            break;
        case ShaderDomain::Surface:
            path_to_base = "/assets/shader/model/shader3D.glsl";
            break;
        case ShaderDomain::Debug:
            path_to_base = "/assets/shader/model/shader3D_line.glsl";
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


