#pragma once

#include "gr-base/core/geometry/default.hpp"
#include <gr-render/shader.hpp>

class MaterialInstance
{
public:
    MaterialInstance(const Material* material);

    inline const Material* GetMaterial() const
    {
        return m_material;
    }

    void SetShader(gr::Shader* shader)
    {
        if (m_shader != shader)
        {
            m_shader = shader;
            isDirty = true;
        }
    }

    void sync();

    void flush();

private:
    const Material* m_material;

    gr::Shader* m_shader;

    bool isDirty;

    std::vector<uint8_t> m_gpuBuffer;

    void buildCache();
};

