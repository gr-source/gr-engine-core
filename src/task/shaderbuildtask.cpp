#include "gr-base/task/shaderbuildtask.hpp"
#include "gr-base/core/geometry/shaderData.hpp"

#include <exception>
#include <stdexcept>
#include <cstring>
#include <fstream>
#include <array>

ShaderBuildTask::ShaderBuildTask(const std::string& file, const std::string& source) : m_file(file), m_source(source)
{}

void ShaderBuildTask::onPreExecute()
{}

int ShaderBuildTask::doInBackground()
{
    try
    {
        auto&& [fragment, vertex] = ReadingSource();

    } catch (const std::exception& e)
    {
        return 1;
    }

    return 0;
}

void ShaderBuildTask::onPostExecute(int result)
{}

std::tuple<std::string, std::string> ShaderBuildTask::ReadingSource()
{
    std::ifstream is(m_source, std::ios_base::in);
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

    return std::make_tuple(mCustomSourceShaderMap[static_cast<int>(ShaderBuildType::FRAGMENT)], mCustomSourceShaderMap[static_cast<int>(ShaderBuildType::VERTEX)]);
}

