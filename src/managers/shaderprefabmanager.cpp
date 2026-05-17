#include "gr-base/managers/shaderprefabmanager.hpp"

ShaderPrefab* ShaderPrefabManager::create(const ShaderAsset* src)
{
    auto&& [it, inserted] = m_prefabmap.emplace(src->GetName(), std::make_unique<ShaderPrefab>(*src));
    if (inserted)
    {
        if (src->GetFlags() & ShaderAssetFlags_Shader3D)
            m_shaders3d.push_back(it->second.get());
    }

    return it->second.get();
}

ShaderPrefab* ShaderPrefabManager::find(const std::string& name)
{
    auto it = m_prefabmap.find(name);
    if (it != m_prefabmap.end())
        return it->second.get();
    return nullptr;
}


