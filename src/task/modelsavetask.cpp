#include "gr-base/task/modelsavetask.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/assetDB.hpp"

#include "gr-base/core/file/memoryStream.hpp"
#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/archetypeStorage.hpp"
#include "gr-base/ecs/world.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"

ModelAsset ModelSaveTask::ConvertToAssetModel(Scene* scene, Entity entity)
{
    auto gameObject = scene->Wrap(entity);

    ModelAsset asset;
    asset.name = scene->GetEntityName(entity);
    asset.isStatic = false;

    auto meshRef = gameObject.GetComponent<MeshRef>();
    if (meshRef != nullptr)
        asset.mesh = meshRef->hash;

    auto materialRef = gameObject.GetComponent<MaterialRef>();
    if (materialRef != nullptr)
    {
        for (size_t i=0;i<materialRef->numMaterials;i++)
            asset.materials.push_back(materialRef->materials[i]);
    }

    for (auto& typeID : scene->GetWorld()->GetAllComponentType(entity))
    {
        auto type = Type::GetTypeInfoByID(typeID);

        auto component = gameObject.GetComponent(typeID);

        std::vector<uint8_t> buffer;
        if (type->serializable)
        {
            buffer.resize(type->stride, 0);

            memcpy(buffer.data(), component, type->stride);
        } else if (type->serialize != nullptr)
        {
            MemoryStream stream;
            ((void (*)(Stream*, const void*))(type->serialize))(&stream, component);

            buffer.resize(stream.tellg(), 0);

            memcpy(buffer.data(), stream.data(), stream.tellg());
        }

        asset.componentBuffer.emplace(type->hash, buffer);
    }

    auto rel = gameObject.GetComponent<Relationship>();
    if (rel != nullptr)
    {
        Entity sibling = rel->firstChild;
        while (sibling != INVALID_ENTITY)
        {
            asset.childrent.push_back(ConvertToAssetModel(scene, sibling));

            auto siblingRel = scene->GetWorld()->GetComponent<Relationship>(sibling);
            if (siblingRel == nullptr)
                break;
            sibling = siblingRel->nextSibling;
        }
    }
    return asset;
}

ModelSaveTask::ModelSaveTask(const std::string& path, const ModelAsset& asset) : m_path(path), m_asset(asset)
{}

ModelSaveTask::ModelSaveTask(const std::string& path, Scene* scene, Entity entity) : m_path(path), m_asset(ConvertToAssetModel(scene, entity))
{}

void ModelSaveTask::onPreExecute()
{
    auto& name = m_asset.name;

    Console::LogInfo("[Task] Saving '%s' model...", name.c_str());
}

// private
std::string ModelSaveTask::doInBackground()
{
    std::string file = gfile::create_temp_file(std::to_string(hash::fnv1_str(m_path.c_str())));

    FileStream stream(file, StreamType::OUTPUT);

    FileHeader header{};
    header.sign = HEADER_MODEL_SIGN;
    header.version = HEADER_ASSET_VERSION;

    if (!stream.write(&header, sizeof(FileHeader)))
        throw std::runtime_error("Falied to write header");

    if (!m_asset.serialize(&stream))
        throw std::runtime_error("Falied to serialize asset");

    stream.close();

    return file;
}

void ModelSaveTask::onPostExecute(Result<std::string> result)
{
    auto name = m_asset.name;

    if (!result.success)
        return Console::LogError("[ModelSaveTask] falied in to save asset '%s'!!!", name.c_str());

    AssetHash hash = hash::fnv1_str(m_path.c_str());

    AssetDB::Registry(hash, m_path);

    std::string dst = AssetDB::Resolve(hash) + HEADER_MODEL_EXTENSION;
    std::string src = result.value;

    if (!gfile::move(src.c_str(), dst.c_str()))
    {
        Console::LogError("[ModelSaveTask] Failed to move file 'asset=%s from=%s to=%s'", name.c_str(), src.c_str(), dst.c_str());

        return AssetDB::Unregistry(hash);
    }

    Console::LogInfo("[ModelSaveTask] Asset saved sucessfully '%s'", name.c_str());
}


