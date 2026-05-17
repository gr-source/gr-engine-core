#include "gr-base/core/geometry/material.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/prefab/shaderprefab.hpp"

#include <gr-render/shader.hpp>

#include <string.h>

Material::Material() : m_prefab(nullptr)
{
}

Material::~Material()
{}

void Material::Set(const std::string &name, const void *data, size_t size)
{
    auto& buffer = m_overrides[name];
    if (buffer.size() != size)
        buffer.resize(size);

    memcpy(buffer.data(), data, size);
}

const void* Material::GetInternal(const std::string &name) const
{
    auto it = m_overrides.find(name);
    if (it == m_overrides.end())
        return nullptr;
    return it->second.data();
}






