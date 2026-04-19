#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/file/default.hpp"

#include <gr-render/gCommon.h>

class MaterialInstance
{
    MaterialID m_materialID;

    gr::Shader* m_shader;

    std::vector<uint8_t> m_parametersBuffer;

    std::unordered_map<std::string, UniformID> m_parameterMap;

public:
    MaterialInstance(MaterialID materialID);

    template <typename T>
    void Set(const char* name, const T& value);

    void Flush() const;

private:
    void Set(const char* name, const void* data, size_t size);
};

template <typename T>
inline void MaterialInstance::Set(const char* name, const T& value)
{
    Set(name, &value, sizeof(T));
}

