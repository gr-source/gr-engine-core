#pragma once

#include "gr-base/core/task/runtime/shaderProperty.hpp"

#include <gr-render/gCommon.h>

#include "default.hpp"

#include <mutex>

class ShaderPropertyManager
{
public:
    ShaderProperty *create(grr::gShader *shader);

    ShaderProperty *get(grr::gShader *shader);

private:
    std::unordered_map<grr::gShader *, std::unique_ptr<ShaderProperty>> m_shaderpropertyma;

    std::mutex m_mutex;
};

