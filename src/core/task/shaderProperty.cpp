#include "gr-base/core/task/shaderProperty.hpp"

ShaderProperty::ShaderProperty()
{
}

void ShaderProperty::addProperty(const std::string &object, const std::string &name, UniformVariable_ variable)
{
    if (m_tabHash.find(name) != m_tabHash.end())
    {
        return;
    }

    mUniformList.emplace_back(object, name, variable);
    m_tabHash.emplace(name);
}

const std::vector<std::tuple<std::string, std::string, UniformVariable_>> &ShaderProperty::getProperty() const
{
    return mUniformList;
}

void ShaderProperty::clear()
{
    mUniformList.clear();
    m_tabHash.clear();    
}
