#include "gr-base/task/modelloadtask.hpp"
#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/file/memoryStream.hpp"
#include "gr-base/core/file/fileStream.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/type.hpp"

void ModelLoadTask::onPreExecute()
{
    Console::LogInfo("[ModelLoadTask] Loading model '%s'...", m_file.c_str());
}

int ModelLoadTask::doInBackground()
{
    FileStream stream(m_file, StreamType::INPUT);
    
    FileHeader header;
    if (!stream.read(&header, sizeof(header)))
        return 1;

    if (header.sign != HEADER_MODEL_SIGN)
        return 2;

    if (header.version != HEADER_ASSET_VERSION)
        return 3;

    if (!m_asset.deserialize(&stream))
        return 4;

    stream.close();
    return 0;
}

void ModelLoadTask::onPostExecute(int result)
{
    if (result != 0)
        return Console::LogError("[ModelLoadTask] Failed to load '%s' error=%d!!!", m_file.c_str(), result);

    auto entity = CreateFromAsset(&m_asset);

    Console::LogInfo("[ModelLoadTask] Model '%s' loaded.", m_asset.name.c_str());

    if (m_func != nullptr)
        m_func(entity);
}

Entity ModelLoadTask::CreateFromAsset(const ModelAsset* asset, Entity parent)
{
    auto go = m_scene->Wrap(m_scene->create(asset->name.c_str()));

    std::cout << asset->name << std::endl;

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

    if (parent != INVALID_ENTITY)
        m_scene->SetParent(parent, go.entity);

    for (auto& child : asset->childrent)
        CreateFromAsset(&child, go.entity);

    return go.entity;
}


