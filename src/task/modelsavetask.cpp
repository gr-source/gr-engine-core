#include "gr-base/task/modelsavetask.hpp"

#include "gr-base/asset/assetResolver.hpp"
#include "gr-base/asset/assetDB.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/file/memoryStream.hpp"
#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/relationship.hpp"

ModelAsset ModelSaveTask::ConvertToAssetModel(const GameObject& gameObject)
{
    ModelAsset asset;
    asset.name = gameObject.GetName();
    asset.isStatic = false;

    /*
    auto meshRef = gameObject.GetComponent<MeshRef>();
    if (meshRef != nullptr)
        asset.mesh = meshRef->hash;

    auto materialRef = gameObject.GetComponent<MaterialRef>();
    if (materialRef != nullptr)
    {
        for (size_t i=0;i<materialRef->numMaterials;i++)
            asset.materials.push_back(materialRef->materials[i]);
    }
    */

    for (auto& typeID : gameObject.GetAllComponentIDs())
    {
        auto type = ComponentRegistry::GetComponentByID(typeID);
        if (type == nullptr)
            continue;

        auto component = gameObject.GetComponentByID(typeID);
        if (component == nullptr)
            continue;

        if (!type->serializable)
            continue;

        std::vector<uint8_t> buffer;
        if (type->isTriviallyCopyable)
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

        if (!buffer.empty())
            asset.componentBuffer.emplace(type->hash, buffer);
    }

    auto rel = gameObject.GetComponent<Relationship>();
    if (rel != nullptr)
    {
        Entity sibling = rel->firstChild;
        while (sibling != INVALID_ENTITY)
        {
            GameObject siblingGo(gameObject.scene, sibling);

            asset.children.push_back(ConvertToAssetModel(siblingGo));

            auto siblingRel = siblingGo.GetComponent<Relationship>();
            if (siblingRel == nullptr)
                break;
            sibling = siblingRel->nextSibling;
        }
    }
    return asset;
}

ModelSaveTask::ModelSaveTask(const URI& uri, const GameObject& gameObject) : m_uri(uri), m_asset(ConvertToAssetModel(gameObject))
{}

void ModelSaveTask::onPreExecute()
{
    auto name = m_uri.toString();

    Console::LogInfo("[ ModelSaveTask ] Saving '%s' model...", name.c_str());
}

// private
std::string ModelSaveTask::doInBackground()
{
    std::string file = gfile::create_temp_file(std::to_string(engine::hash::fnv1_str(m_uri.toString())));

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
    /*
    auto name = m_uri.toString();

    if (!result.success)
        return Console::LogError("[ModelSaveTask] falied in to save asset '%s'!!!", name.c_str());

    std::string realPath = AssetResolver::Resolve(m_uri);
    {
        if (gfile::exists(realPath) && !gfile::is_directory(realPath))
            gfile::remove(realPath);

        std::string parent = gfile::parent(realPath);

        if (!gfile::exists(parent) && !gfile::create_directories(parent))
            return;
    }

    std::string dst = AssetResolver::Resolve(m_uri) + HEADER_MODEL_EXTENSION;
    if (gfile::exists(dst))
        gfile::remove(dst);

    std::string src = result.value;

    if (!gfile::move(src.c_str(), dst.c_str()))
    {
        Console::LogError("[ModelSaveTask] Failed to move file 'asset=%s from=%s to=%s'", name.c_str(), src.c_str(), dst.c_str());

        return;
    }

    AssetDB::Registry(name, AssetFlags_AssetModel);

    Console::LogInfo("[ModelSaveTask] Asset saved sucessfully '%s'", name.c_str());
    */
}


