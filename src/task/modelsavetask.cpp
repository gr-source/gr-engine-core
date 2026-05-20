#include "gr-base/task/modelsavetask.hpp"

#include "gr-base/core/assetReference.hpp"
#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/file/memoryStream.hpp"
#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/gameObject.hpp"

#include "gr-base/script/transform.hpp"

ModelAsset ModelSaveTask::ConvertToAssetModel(Handle handle)
{
    // auto gameObject = GameObject::Get(handle);
    //
    // ModelAsset asset;
    // asset.name = gameObject->GetName();
    // asset.isStatic = gameObject->GetIsStatic();

    /*
    for (auto& hash : gameObject->GetHashes())
    {
        Component* behavior = gameObject->getComponent(hash);

        MemoryStream stream;
        behavior->Serialize(&stream);

        std::vector<uint8_t> buffer(stream.tellg(), 0);
        memcpy(buffer.data(), stream.data(), stream.tellg());

        asset.componentBuffer.emplace(hash, buffer);
    }
    */

    // auto transform = gameObject->GetTransform();
    /*
    for (auto& childID : transform->getChildren())
    {
        auto childTransform = ComponentStorage::GetComponent<Transform>(childID);

        asset.childrent.push_back(ConvertToAssetModel(childTransform->GetGameObject()->GetCacheID()));
    }
    */

    // return asset;
}

ModelSaveTask::ModelSaveTask(const std::string& file, const ModelAsset& asset) : m_file(file), m_asset(asset)
{}


ModelSaveTask::ModelSaveTask(const std::string& file, Handle handle) : m_file(file), m_asset(ConvertToAssetModel(handle))
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


