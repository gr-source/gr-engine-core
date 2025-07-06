#include "gr-base/core/geometry/shaderpropertymanager.hpp"

ShaderProperty *ShaderPropertyManager::create(grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(m_mutex);
}

ShaderProperty *ShaderPropertyManager::get(grr::gShader *shader)
{}

