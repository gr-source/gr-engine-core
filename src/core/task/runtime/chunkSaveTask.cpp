#include "gr-base/core/task/runtime/chunkSaveTask.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/memoryStream.hpp"
#include "gr-base/core/gameObject.hpp"
#include "gr-base/ecs/behaviorStorage.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/managers/chunkmanager.hpp"
#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"

ChunkSaveTask::ChunkSaveTask(ChunkID chunkID, std::function<void (ChunkID)> func) : m_chunkID(chunkID), m_function(func)
{}

// private
int ChunkSaveTask::doInBackground()
{
    StreamChunk* chunk = ChunkManager::Get(m_chunkID);

    FileStream stream("/home/NoirVoid/gr-engine/" + std::string(chunk->name) + ".grck", StreamType::OUTPUT);

    FileHeader header{{'g', 'r', 'c', 'k'}, 1.0f};
    if (!stream.write(&header, sizeof(FileHeader)))
        return 1;

    if (write_metadata(&stream) != 0)
        return 2;

    if (write_data(&stream) != 0)
        return 3;

    stream.close();

    return 0;
}

void ChunkSaveTask::onPostExecute(int result)
{
    if (result != 0)
        return Console::LogError("[ ChunkSaveTask ] Fail to save chunk: '%d'", result);

    Console::LogInfo("[ ChunkSaveTask ] Chunk writed sucess fully!");

    if (m_function)
        m_function(m_chunkID);
}

int ChunkSaveTask::write_segment(Stream* stream, HeaderType type, std::function<int(Stream*)> func)
{
    size_t first = stream->tellp();

    DataSegment dataSegment = {type, 0};
    if (!stream->write(&dataSegment, sizeof(DataSegment)))
        return 1;

    if (func != nullptr && func(stream) != 0)
        return 2;

    size_t current = stream->tellp();

    stream->seekp(first);

    dataSegment = {type, current - first - sizeof(DataSegment)};
    if (!stream->write(&dataSegment, sizeof(DataSegment)))
        return 2;

    stream->seekp(current);
    return 0;
}

int ChunkSaveTask::write_data(Stream *stream)
{
    // shader
    if (write_segment(stream, HeaderType::SHADER, std::bind(&ChunkSaveTask::write_shader, this, std::placeholders::_1)))
        return 1;

    // textures
    if (write_segment(stream, HeaderType::TEXTURE, std::bind(&ChunkSaveTask::write_texture, this, std::placeholders::_1)))
        return 2;

    // material
    if (write_segment(stream, HeaderType::MATERIAL, std::bind(&ChunkSaveTask::write_material, this, std::placeholders::_1)))
        return 3;

    // Mesh
    if (write_segment(stream, HeaderType::MESH, std::bind(&ChunkSaveTask::write_mesh, this, std::placeholders::_1)))
        return 4;

    // Objects
    if (write_segment(stream, HeaderType::OBJECT, std::bind(&ChunkSaveTask::write_gameobject, this, std::placeholders::_1)))
        return 5;

    // script
    if (write_segment(stream, HeaderType::SCRIPT, std::bind(&ChunkSaveTask::write_script, this, std::placeholders::_1)))
        return 6;

    // end file
    if (write_segment(stream, HeaderType::NONE, nullptr))
        return 7;

    return 0;
}


int ChunkSaveTask::write_metadata(Stream *stream)
{
    read_gameobject();

    return 0;
}

int ChunkSaveTask::write_shader(Stream *stream)
{
    auto length = m_shadermap.size();
    if (!stream->write(length))
    {
        return 1;
    }

    for (auto &shadername : m_shadermap)
    {
        if (!stream->write(shadername))
        {
            return 2;
        }
    }
    return 0;
}

int ChunkSaveTask::write_texture(Stream *stream)
{
    return 0;
}

int ChunkSaveTask::write_material(Stream *stream)
{
    auto length = m_materialmap.size();
    if (!stream->write(length))
    {
        return 1;
    }

    for (auto &&[name, materialID] : m_materialmap)
    {
        auto material = AssetsManager::GetMaterial(materialID);

        if (!stream->write(name))
        {
            return 2;
        }

        auto shadername = AssetsManager::GetShaderName(material->GetShader(false));
        if (!stream->write(shadername))
        {
            return 3;
        }

        // deserialize material
        std::size_t bufferSize = 0;
        {
            MemoryStream temp;
            material->serialize(&temp);
            bufferSize = temp.tellg();

            temp.close();
        }

        if (!stream->write(bufferSize))
        {
            return 4;
        }

        material->serialize(stream);
    }
    return 0;
}

int ChunkSaveTask::write_mesh(Stream *stream)
{
    auto size = m_meshmap.size();
    if (!stream->write(size))
        return 1;

    for (auto &meshID : m_meshmap)
    {
        auto name = AssetsManager::GetMeshName(meshID);
        if (!stream->write(name))
            return 2;

        Mesh* mesh = AssetsManager::GetMesh(meshID);

        MemoryStream temp;
        mesh->serialize(&temp);

        std::size_t length = temp.tellg();
        if (!stream->write(length))
            return 3;

        mesh->serialize(stream);
    }
    return 0;
}

int ChunkSaveTask::write_gameobject(Stream *stream)
{
    StreamChunk* chunk = ChunkManager::Get(m_chunkID);

    GameObjectHandle* objects = chunk->objects;

    size_t length = vector_length(objects);
    if (!stream->write(length))
        return 1;

    for (size_t i=0;i<length;i++)
    {
        auto gameObject = GameObject::Get(objects[i]);
        if (!serialize(stream, gameObject))
            return 2;
    }

    return 0;
}

int ChunkSaveTask::write_script(Stream *stream)
{
    {
        auto count = m_componentdata.size();
        if (!stream->write(count))
            return 1;
    }

    for (auto&& [name, behaviorList] : m_componentdata)
    {
        if (!stream->write(name))
            return 2;

        size_t startbuffer = stream->tellp();

        uint64_t buffersize = 0;
        if (!stream->write(buffersize))
            return 3;

        uint32_t count = static_cast<uint32_t>(behaviorList.size());
        if (!stream->write(count))
            return 4;

        for (auto&& [hash, behavior] : behaviorList)
        {
            if (!stream->write(hash))
                return 4;

            size_t bufferlength = {};
            {
                MemoryStream temp;
                behavior->Serialize(&temp);
                bufferlength = temp.tellg();
                temp.close();
            }

            if (!stream->write(bufferlength))
                return 5;

            if (bufferlength > 0)
                behavior->Serialize(stream);
        }

        size_t current = stream->tellp();

        buffersize = static_cast<uint64_t>(current - startbuffer - sizeof(uint64_t));
        
        stream->seekp(startbuffer);
        if (!stream->write(buffersize))
            return 6;

        stream->seekp(current);
    }
    return 0;
}

void ChunkSaveTask::read_gameobject()
{
    StreamChunk* chunk = ChunkManager::Get(m_chunkID);

    GameObjectHandle* objects = chunk->objects;

    size_t length = vector_length(objects);
    for (size_t i=0;i<length;i++)
    {
        auto gameObject = GameObject::Get(objects[i]);

        read_data(gameObject);
    }
}

void ChunkSaveTask::read_data(const GameObject *gameObject)
{
    std::vector<std::tuple<uint64_t, const Behavior*>> behaviorList;
    for (uint64_t hash : gameObject->GetHashes())
        behaviorList.emplace_back(hash, gameObject->getComponent(hash));

    m_componentdata.emplace_back(gameObject->GetName(), behaviorList);

    auto meshFilter = gameObject->getComponent<MeshFilter>();
    if (meshFilter != nullptr)
    {
        for (auto &meshID : meshFilter->getMesh())
            m_meshmap.emplace(meshID);
    }

    if (gameObject->getComponent<MeshRender>() || gameObject->getComponent<SkinnedMeshRender>())
    {
        auto baseRender = gameObject->getComponent<MeshRender>() ? static_cast<BaseRender *>(gameObject->getComponent<MeshRender>()) : static_cast<BaseRender *>(gameObject->getComponent<SkinnedMeshRender>());
        
        for (auto materialID : baseRender->getMaterialIDs())
        {
            auto material_name = AssetsManager::GetMaterialName(materialID);
            auto material = AssetsManager::GetMaterial(materialID);

            auto shader_name = AssetsManager::GetShaderName(material->GetShader(false));

            {
                auto it = m_shadermap.find(shader_name);
                if (it == m_shadermap.end())
                    m_shadermap.emplace(shader_name);
            }

            {
                auto it = m_materialmap.find(material_name);
                if (it == m_materialmap.end())
                    m_materialmap.emplace(material_name, materialID);
            }
        }
    }

    auto transform = gameObject->GetTransform();
    for (auto &child : transform->getChildren())
        read_data(BehaviorStorage::GetComponent<Transform>(child)->GetGameObject());
}

bool ChunkSaveTask::serialize(Stream *stream, const GameObject *go)
{
    if (!stream->write(go->GetName()))
        return false;

    if (!stream->write(go->GetIsStatic()))
        return false;

    auto transform = go->GetTransform();

    auto children = transform->getChildren();

    auto ccount = children.size();
    if (!stream->write(ccount))
    {
        return false;
    }

    for (auto &child : children)
    {
        if (!serialize(stream, BehaviorStorage::GetComponent<Transform>(child)->GetGameObject()))
        {
            return false;
        }
    }
    
    return true;
}




