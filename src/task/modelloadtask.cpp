#include "gr-base/task/modelloadtask.hpp"
#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/memoryStream.hpp"
#include "gr-base/core/gameObject.hpp"

#include "gr-base/script/transform.hpp"

void ModelLoadTask::onPreExecute()
{
    Console::LogInfo("[Task] Loading model '%s'...", m_file.c_str());
}

int ModelLoadTask::doInBackground()
{
    FileStream stream(m_file, StreamType::INPUT);
    
    FileHeader header;
    if (!stream.read(&header, sizeof(header)))
        return 1;

    if (strcmp(header.sign, "gmdl") != 0)
        return 2;

    if (!m_asset.deserialize(&stream))
        return 3;

    stream.close();
    return 0;
}

void ModelLoadTask::onPostExecute(int result)
{
    if (result != 0)
    {
        Console::LogError("[Task] Failed to load '%s'!!!", m_file.c_str());
        return;
    }

    auto handle = CreateFromAsset(&m_asset);

    Console::LogInfo("[Task] Model '%s' loaded.", m_asset.name.c_str());

    if (m_func != nullptr)
        m_func(handle);
}

GameObjectHandle ModelLoadTask::CreateFromAsset(const ModelAsset* asset, GameObjectHandle parent)
{
    GameObjectHandle handle = GameObject::Create(asset->name);

    auto gameObject = GameObject::Get(handle);

    auto transform = gameObject->GetTransform();

    for (auto &&[hash, buffer] : asset->componentBuffer)
    {
        Component* behavior = gameObject->getComponent(hash);
        if (behavior == nullptr)
            behavior = gameObject->addComponent(hash);

        if (behavior != nullptr)
        {
            MemoryStream stream((uint8_t*)buffer.data(), buffer.size());
            behavior->Deserialize(&stream);
        }
    }

    if (parent != HANDLE_INVALID)
    {
        auto gameObjectParent = GameObject::Get(parent);

        auto transformParent = gameObjectParent->GetTransform();
        transformParent->addChildren(transform);
    }

    for (auto& child : asset->childrent)
        CreateFromAsset(&child, handle);

    return handle;
}


