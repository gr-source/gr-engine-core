#include "gr-base/task/modelsavetask.hpp"

#include "gr-base/core/assetReference.hpp"
#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/file/memoryStream.hpp"
#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/archetypeStorage.hpp"
#include "gr-base/ecs/ecs.hpp"

#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"

ModelAsset ModelSaveTask::ConvertToAssetModel(Scene* scene, Entity entity)
{
    auto gameObject = scene->Wrap(entity);

    ModelAsset asset;
    asset.name = scene->GetEntityName(entity);
    asset.isStatic = false/* gameObject->GetIsStatic() */;

    for (auto& typeID : scene->GetECS()->GetAllComponentType(entity))
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

            auto siblingRel = scene->GetECS()->GetComponent<Relationship>(sibling);
            if (siblingRel == nullptr)
                break;
            sibling = siblingRel->nextSibling;
        }
    }
    return asset;
}

ModelSaveTask::ModelSaveTask(const std::string& file, const ModelAsset& asset) : m_file(file), m_asset(asset)
{}

ModelSaveTask::ModelSaveTask(const std::string& file, Scene* scene, Entity entity) : m_file(file), m_asset(ConvertToAssetModel(scene, entity))
{}

void ModelSaveTask::onPreExecute()
{
    auto& name = m_asset.name;

    Console::LogInfo("[Task] Saving '%s' model...", name.c_str());
}

// private
int ModelSaveTask::doInBackground()
{
    FileStream stream(m_file, StreamType::OUTPUT);

    FileHeader header{};
    header.sign = HEADER_MODEL_SIGN;
    header.version = HEADER_ASSET_VERSION;

    if (!stream.write(&header, sizeof(FileHeader)))
        return 1;

    if (!m_asset.serialize(&stream))
        return 2;

    stream.close();
    return 0;
}

void ModelSaveTask::onPostExecute(int result)
{
    auto& name = m_asset.name;

    if (result != 0)
        return Console::LogError("[Task] Falied in to save '%s'!!!", name.c_str());

    Console::LogInfo("[Taks] Model '%s' saved in '%s'.", name.c_str(), m_file.c_str());

    AssetReference::Registry(name, m_file);
}


