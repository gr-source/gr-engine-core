#include "gr-base/core/geometry/material.hpp"

#include <gr-render/shader.hpp>

#include <string.h>

void Material::Set(const std::string &name, const void *data, size_t size)
{
    auto& buffer = overrides[name];
    if (buffer.size() != size)
        buffer.resize(size);

    memcpy(buffer.data(), data, size);
}

const void* Material::GetInternal(const std::string &name) const
{
    auto it = overrides.find(name);
    if (it == overrides.end())
        return nullptr;
    return it->second.data();
}

void Material::Build()
{
    buffer.resize(shader->GetUniformBufferSize());

    for (size_t i=0;i<shader->GetUniformCount();i++)
    {
        auto& uniform = shader->GetUniforms()[i];

        const void* data = GetInternal(uniform.name);
        if (data != nullptr)
            memcpy(buffer.data() + uniform.offset, data, uniform.stride);
    }
}

void Material::Flush()
{
    if (buffer.empty())
        return;

    auto base = buffer.data();

    auto uniforms = shader->GetUniforms();
    for (UniformID id=0;id<shader->GetUniformCount();id++)
    {
        auto& uniform = uniforms[id];

        if (strstr(uniform.name, "u_mat_") != nullptr)
            continue;

        shader->SetUniform(id, base + uniform.offset);
    }
}






