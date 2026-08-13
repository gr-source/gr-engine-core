#include "gr-base/core/geometry/material.hpp"

#include <gr-render/gl.h>
#include <gr-render/shader.hpp>

#include <string.h>

void Material::Set(const std::string &name, const void *data, size_t size)
{
    auto& buffer = overrides[name];
    if (size > buffer.size())
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

void Material::Sync()
{
    if (shader == lastShader)
        return;

    lastShader = shader;

    Build();
}

void Material::Build()
{
    if (shader->bufferSize >= GPUbuffer.size())
    {
        GPUbuffer.resize(shader->bufferSize + 1);

        memset(GPUbuffer.data(), 0, shader->bufferSize);
    } 

    for (size_t i=0;i<shader->count;i++)
    {
        auto& uniform = shader->uniforms[i];

        const void* src = GetInternal(uniform.name);
        if (src != nullptr)
            memcpy(&GPUbuffer[uniform.offset], src, uniform.size);
    }
}

void Material::Flush() const
{
    if (GPUbuffer.empty())
        return;

    size_t textureActive = 0;

    for (size_t i=0;i<shader->count;i++)
    {
        auto& uniform = shader->uniforms[i];

        // if (strstr(uniform.name, "u_mat_") != nullptr)
        //     break;

        const void* data = static_cast<const void*>(&GPUbuffer[uniform.offset]);

        if (uniform.type == gr::UniformType::SAMPLER2D)
        {
            int textureID = *(const int*)(data);
            int active = textureActive++;

            glActiveTexture(GL_TEXTURE0 + active);
            glBindTexture(GL_TEXTURE_2D, textureID);

            shader->SetUniform(i, &active);
        } else
        {
            shader->SetUniform(i, data);
        }
    }
}






