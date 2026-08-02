#include "gr-base/task/modelloadtask.hpp"

#include "gr-base/asset/assetDB.hpp"
#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/file/memoryStream.hpp"
#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/spatial/bvhtree.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include <cerrno>
#include <cstring>
#include <exception>

void ModelLoadTask::onPreExecute()
{
    Console::LogInfo("[ModelLoadTask] Loading model '%s'...", m_file.c_str());
}

ModelAsset ModelLoadTask::doInBackground()
{
    if (!gfile::exists(m_file))
        throw std::runtime_error(strerror(ENOENT));

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
    GameObject go(m_scene, m_scene->create(asset->name.c_str()));

    for (auto &&[hash, buffer] : asset->componentBuffer)
    {
        auto type = ComponentRegistry::GetComponentInfoByHash(hash);
        if (type == nullptr)
            continue;

        void* component = go.GetOrAddComponentByID(type->index);
        if (component == nullptr)
            continue;

        if (!type->serializable)
            continue;

        if (type->isTriviallyCopyable)
        {
            memcpy(component, buffer.data(), type->stride);
        } else if (type->deserialize != nullptr)
        {
            MemoryStream stream((uint8_t*)buffer.data(), buffer.size());

            ((void (*)(Stream*, void*))(type->deserialize))(&stream, component);
        }
    }

    if (asset->mesh != INVALID_ASSET)
    {
        try
        {
            MeshHandle mesh =
                AssetsManager::LoadMesh(asset->mesh);

            if (mesh != HANDLE_INVALID)
            {
                auto meshFilter = go.GetOrAddComponent<MeshFilter>();
                meshFilter->mesh = mesh;
            }

        } catch (const std::exception& e)
        {
            Console::LogWarning("[ ModelLoadTask ] Mesh não carregada asset='%ld' error='%s'. ", asset->mesh, e.what());
        }

        try
        {

            std::vector<MaterialHandle> materials;
            for (auto& assetID : asset->materials)
            {
                MaterialHandle material =
                    AssetsManager::LoadMaterial(assetID);

                materials.push_back(material);
            }

            auto meshRender = go.GetOrAddComponent<MeshRender>();
            meshRender->materials = (MaterialHandle*)malloc(materials.size() * sizeof(MaterialHandle));
            meshRender->numMaterials = materials.size();

            memcpy(meshRender->materials, materials.data(), materials.size() * sizeof(MaterialHandle));

            /*
            auto materialRef = go.GetOrAddComponent<MaterialRef>();
            if (materialRef != nullptr)
            {
                materialRef->materials = (AssetHash*)malloc(asset->materials.size() * sizeof(AssetHash));
                materialRef->numMaterials = asset->materials.size();

                memcpy(materialRef->materials, asset->materials.data(), asset->materials.size() * sizeof(AssetHash));
            }
            */
        } catch (const std::exception& e)
        {
            Console::LogWarning("[ ModelLoadTask ] Material não carregado: error='%s'.", e.what());
        }
    }

    if (go.HasComponent<MeshFilter, MeshRender>())
    {
        auto meshFilter = go.GetComponent<MeshFilter>();
        if (meshFilter->mesh != HANDLE_INVALID)
        {
            auto bvh = m_scene->GetBVHtree();

            auto mesh = AssetsManager::GetMesh(meshFilter->mesh);
            if (mesh != nullptr)
            {
                auto& bounds = mesh->localBounds;
                if (bounds.Volume() > 0.0f)
                    go.AddComponent<RenderBounds>(bounds, bvh->insert(go.entity, bounds));
            }
        }
    }

    for (auto& assetChild : asset->children)
    {
        Entity child = CreateFromAsset(&assetChild);

        m_scene->SetParent(go.entity, child);
    }

    return go.entity;
}



