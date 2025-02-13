#include "gr-base/core/task/shaderProperty.hpp"

ShaderProperty::ShaderProperty()
{
}

void ShaderProperty::addProperty(const std::string &object, const std::string &name, UniformType type)
{
    mUniformList.emplace_back(object, name, type);
}

const std::vector<std::tuple<std::string, std::string, UniformType>> &ShaderProperty::getProperty() const
{
    return mUniformList;
}

void ShaderProperty::clear()
{
    mUniformList.clear();
}
