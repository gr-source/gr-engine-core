#include "gr-base/task/modelloadtask.hpp"
#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/file/memoryStream.hpp"
#include "gr-base/core/file/fileStream.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/type.hpp"
#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"

void ModelLoadTask::onPreExecute()
{
    Console::LogInfo("[ModelLoadTask] Loading model '%s'...", m_file.c_str());
}

ModelAsset ModelLoadTask::doInBackground()
{
    FileStream stream(m_file, StreamType::INPUT);
    
    FileHeader header;
    if (!stream.read(&header, sizeof(header)))
        throw std::runtime_error("Falied to read header");

    if (header.sign != HEADER_MODEL_SIGN)
        throw std::runtime_error("Invalid asset signature");

    if (header.version != HEADER_ASSET_VERSION)
        throw std::runtime_error("Asset version is'n compatible");

    ModelAsset asset;
    if (!asset.deserialize(&stream))
        throw std::runtime_error("Falied to deserialize asset");

    stream.close();
    return asset;
}

void ModelLoadTask::onPostExecute(Result<ModelAsset> result)
{
    if (!result.success)
        return Console::LogError("[ModelLoadTask] Failed to load '%s' error=%s!!!", m_file.c_str(), result.error.c_str());

    auto entity = CreateFromAsset(&result.value);

    Console::LogInfo("[ModelLoadTask] Model '%s' loaded.", result.value.name.c_str());

    if (m_func != nullptr)
        m_func(entity);
}

Entity ModelLoadTask::CreateFromAsset(const ModelAsset* asset)
{
    auto go = m_scene->Wrap(m_scene->create(asset->name.c_str()));

    for (auto &&[hash, buffer] : asset->componentBuffer)
    {
        auto type = Type::GetTypeInfoByHash(hash);
        if (type == nullptr)
            continue;

        auto component = go.GetOrAddComponent(type->index);
        if (type->serializable)
        {
            if (type->globalCpy)
                memcpy(component, buffer.data(), type->stride);
        } else if (type->deserialize != nullptr)
        {
            MemoryStream stream((uint8_t*)buffer.data(), buffer.size());

            ((void (*)(Stream*, void*))(type->deserialize))(&stream, component);
        }
    }

    if (asset->mesh != INVALID_HASH64)
    {
        MeshHandle mesh = AssetsManager::FindMesh(asset->mesh);
        if (mesh == HANDLE_INVALID)
            mesh = AssetsManager::LoadMesh(asset->mesh);

        {
            auto meshFilter = go.GetOrAddComponent<MeshFilter>();
            meshFilter->mesh = mesh;
        }

        {
            auto meshRef = go.GetOrAddComponent<MeshRef>();
            meshRef->hash = asset->mesh;
        }

        std::vector<MaterialHandle> materials;
        for (auto& key : asset->materials)
        {
            MaterialHandle material = AssetsManager::FindMaterial(key);
            if (material == HANDLE_INVALID)
                material = AssetsManager::LoadMaterial(key);

            materials.push_back(material);
        }

        auto meshRender = go.GetOrAddComponent<MeshRender>();
        meshRender->materials = (MaterialHandle*)malloc(materials.size() * sizeof(MaterialHandle));
        meshRender->numMaterials = materials.size();

        memcpy(meshRender->materials, materials.data(), materials.size() * sizeof(MaterialHandle));

        auto materialRef = go.GetOrAddComponent<MaterialRef>();
        if (materialRef != nullptr)
        {
            materialRef->materials = (MaterialHandle*)malloc(asset->materials.size() * sizeof(MaterialHandle));
            materialRef->numMaterials = asset->materials.size();

            memcpy(materialRef->materials, asset->materials.data(), asset->materials.size() * sizeof(MaterialHandle));
        }
    }

    for (auto& assetChild : asset->childrent)
    {
        Entity child = CreateFromAsset(&assetChild);

        m_scene->SetParent(go.entity, child);
    }

    return go.entity;
}


