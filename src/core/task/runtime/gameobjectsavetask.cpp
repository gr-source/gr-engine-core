#include "gr-base/core/task/runtime/gameobjectsavetask.hpp"

#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/memoryStream.hpp"
#include "gr-base/ecs/behaviorStorage.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/geometry/material.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/script/LODComponent.hpp"
#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"
#include <functional>

GameObjectSaveTask::GameObjectSaveTask(const std::string &file, GameObjectHandle gameObjectID, std::function<void (GameObjectHandle)> func) : m_file(file), m_gameObjectID(gameObjectID), m_func(func)
{}

// private
int GameObjectSaveTask::doInBackground()
{
    auto gameObject = GameObject::Get(m_gameObjectID);

    FileStream stream(m_file, StreamType::OUTPUT);

    // Header file
    {
        FileHeader header{{'g', 'm', 'd', 'x'}, 1.1f};
        if (!stream.write(&header, sizeof(FileHeader)))
        {
            Console::LogError("[ GameObjectSaveTask ] Fail to write header.");
            return 1;
        }
    }

    if (write_metadata(&stream) != 0)
        return 2;

    int result = write_data(&stream);
    if (result != 0)
        return 3;

    stream.close();

    return 0;
}

void GameObjectSaveTask::onPostExecute(int result)
{
    if (result != 0)
    {
        return Console::LogError("[ GameObjectSaveTask ] Fail to save gameObject");
    }

    auto gameObject = GameObject::Get(m_gameObjectID);

    Console::LogInfo("[ GameObjectSaveTask ] Saved: %s", gameObject->getName().c_str());

    if (m_func != nullptr)
        m_func(m_gameObjectID);
}

int GameObjectSaveTask::write_metadata(Stream *stream)
{
    read_gameobject(GameObject::Get(m_gameObjectID));
    return 0;
}

int GameObjectSaveTask::write_segment(Stream* stream, HeaderType type, std::function<int(Stream*)> func)
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

int GameObjectSaveTask::write_data(Stream *stream)
{
    // write texture
    if (write_segment(stream, HeaderType::TEXTURE, std::bind(&GameObjectSaveTask::write_texture, this, std::placeholders::_1)))
        return 1;

    // write material
    if (write_segment(stream, HeaderType::MATERIAL, std::bind(&GameObjectSaveTask::write_material, this, std::placeholders::_1)))
        return 2;

    // write mesh
    if (write_segment(stream, HeaderType::MESH, std::bind(&GameObjectSaveTask::write_mesh, this, std::placeholders::_1)))
        return 3;

    // write gameobject
    if (write_segment(stream, HeaderType::OBJECT, std::bind(&GameObjectSaveTask::write_gameobject, this, std::placeholders::_1, GameObject::Get(m_gameObjectID))))
        return 5;

    // write script
    if (write_segment(stream, HeaderType::SCRIPT, std::bind(&GameObjectSaveTask::write_script, this, std::placeholders::_1)))
        return 6;

    // end file
    if (write_segment(stream, HeaderType::NONE, nullptr))
        return 7;

    return 0;
}

/*
int GameObjectSaveTask::write_shader(Stream *stream)
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
*/

int GameObjectSaveTask::write_texture(Stream *stream)
{
    /*
    auto textureCount = m_textureMap.size();
    if (!stream->write(&textureCount, sizeof(std::size_t)))
    {
        return 1;
    }

    for (auto [goname, mname, tpath] : m_textureMap)
    {
        // write game object name
        {
            auto gonsize = goname.size();
            if (!stream->write(&gonsize, sizeof(std::size_t)))
            {
                return 2;
            }

            if (!stream->write(goname.c_str(), gonsize * sizeof(char)))
            {
                return 3;
            }
        }

        // write material name
        {
            auto mnsize = mname.size();
            if (!stream->write(&mnsize, sizeof(std::size_t)))
            {
                return 4;
            }

            if (!stream->write(mname.c_str(), mnsize * sizeof(char)))
            {
                return 5;
            }
        }

        // write texture path
        {
            auto tpsize = tpath.size();
            if (!stream->write(&tpsize, sizeof(std::size_t)))
            {
                return 4;
            }

            if (!stream->write(tpath.c_str(), tpsize * sizeof(char)))
            {
                return 5;
            }
        }
    }
    */
    return 0;
}

int GameObjectSaveTask::write_material(Stream *stream)
{
    auto length = m_materialmap.size();
    if (!stream->write(length))
        return 1;

    for (auto &&[name, materialID] : m_materialmap)
    {
        auto material = AssetsManager::GetMaterial(materialID);

        if (!stream->write(name))
            return 2;

        // serialize material
        std::size_t bufferSize = 0;
        {
            MemoryStream temp;
            material->serialize(&temp);
            bufferSize = temp.tellg();

            temp.close();
        }

        if (!stream->write(bufferSize))
            return 4;

        material->serialize(stream);
    }
    return 0;
}

int GameObjectSaveTask::write_mesh(Stream *stream)
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

int GameObjectSaveTask::write_gameobject(Stream *stream, const GameObject *go)
{
    if (!stream->write(go->getName()))
        return 1;

    if (!stream->write(go->GetIsStatic()))
        return 2;

    auto transform = go->GetTransform();

    auto children = transform->getChildren();

    auto ccount = children.size();
    if (!stream->write(ccount))
        return 3;

    for (auto &child : children)
    {
        int result = write_gameobject(stream, BehaviorStorage::GetComponent<Transform>(child)->GetGameObject());
        if (result != 0)
            return 4;
    }

    return 0;
}

int GameObjectSaveTask::write_script(Stream *stream)
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

void GameObjectSaveTask::read_gameobject(const GameObject *gameObject)
{
    std::vector<std::tuple<uint64_t, const Behavior*>> behaviorList;
    for (uint64_t hash : gameObject->GetHashes())
        behaviorList.emplace_back(hash, gameObject->getComponent(hash));

    m_componentdata.emplace_back(gameObject->getName(), behaviorList);

    auto meshFilter = gameObject->getComponent<MeshFilter>();
    if (meshFilter != nullptr)
    {
        for (auto &meshID : meshFilter->getMesh())
            m_meshmap.emplace(meshID);
    }


    auto lodComponent = gameObject->getComponent<LODComponent>();
    if (lodComponent != nullptr)
    {
        for (auto &lod : lodComponent->meshesLOD)
        {
            for (auto& meshID : lod.meshes)
                m_meshmap.emplace(meshID);
        }
    }

    if (gameObject->getComponent<MeshRender>() || gameObject->getComponent<SkinnedMeshRender>())
    {
        auto baseRender = gameObject->getComponent<MeshRender>() ? static_cast<BaseRender *>(gameObject->getComponent<MeshRender>()) : static_cast<BaseRender *>(gameObject->getComponent<SkinnedMeshRender>());
        
        for (auto materialID : baseRender->getMaterialIDs())
        {
            auto material_name = AssetsManager::GetMaterialName(materialID);
            auto material = AssetsManager::GetMaterial(materialID);

            {
                auto it = m_materialmap.find(material_name);
                if (it == m_materialmap.end())
                    m_materialmap.emplace(material_name, materialID);
            }
        }
    }

    auto transform = gameObject->GetTransform();
    for (auto &child : transform->getChildren())
        read_gameobject(BehaviorStorage::GetComponent<Transform>(child)->GetGameObject());
}



