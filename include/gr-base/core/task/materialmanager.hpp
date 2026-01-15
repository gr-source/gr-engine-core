#pragma once

#include "gr-base/core/task/runtime/default.hpp"

#include "gr-base/core/geometry/material.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <stack>

class MaterialManager
{
public:
    MaterialManager();

    MaterialID create(grr::gShader *shader, const std::string &name);

    Material &get(MaterialID id);

    MaterialID find(const std::string &name);

    const char *getname(MaterialID id);

    void remove(MaterialID id);

private:
    std::unordered_map<std::string, MaterialID> m_materialnamemap;

    std::unordered_map<MaterialID, std::string> m_materialmap;

    std::vector<Material> m_materials;

    std::stack<MaterialID> m_freeIDs;

    std::mutex m_mutex;

    void resize(std::size_t size);

    void addfreelist(MaterialID id);
};


