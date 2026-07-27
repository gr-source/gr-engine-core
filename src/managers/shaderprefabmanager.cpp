#include "gr-base/managers/shaderprefabmanager.hpp"

ShaderPrefab* ShaderPrefabManager::create(AssetID id, const ShaderAsset* src)
{
    auto&& [it, inserted] = m_prefabmap.emplace(id, std::make_unique<ShaderPrefab>(*src));
    if (inserted)
    {
        // if (src->flags & ShaderAssetFlags_Shader3D)
        //     m_shaders3d.push_back(it->second.get());
    }

    return it->second.get();
}

ShaderPrefab* ShaderPrefabManager::find(AssetID id) const
{
    auto it = m_prefabmap.find(id);
    if (it != m_prefabmap.end())
        return it->second.get();
    return nullptr;
}


