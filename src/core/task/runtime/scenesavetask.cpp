#include "gr-base/core/task/runtime/scenesavetask.hpp"

#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/memoryStream.hpp"

#include "gr-base/ecs/componentstorage.hpp"

#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/sceneManager.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"
#include <functional>

SceneSaveTask::SceneSaveTask(const std::string &file, const Scene *scene, std::function<void ()> func) : m_file(file), m_scene(scene), m_func(func)
{}

// ***** private ***** //
int SceneSaveTask::doInBackground()
{
    Console::LogInfo("[ SceneSaveTask ] Writing scene %s...", m_file.c_str());

    FileStream stream(m_file, StreamType::OUTPUT);
    // header data
    {
        FileHeader header{};
        header.sign = HEADER_SCENE_SIGN;
        header.version = HEADER_ASSET_VERSION;

        if (!stream.write(&header, sizeof(FileHeader)))
            return 1;
    }

    if (write_metadata(&stream) != 0)
        return 2;

    if (write_data(&stream) != 0)
        return 3;

    return 0;
}

void SceneSaveTask::onPostExecute(int result)
{
    if (result != 0)
        return Console::LogError("[ SceneSaveTask ] Fail to write scene, code: %d", result);

    Console::LogInfo("[ SceneSaveTask ] Scene writed sucess fully!");

    if (m_func != nullptr)
        m_func();
}

int SceneSaveTask::write_segment(Stream* stream, HeaderType type, std::function<int(Stream*)> func)
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

int SceneSaveTask::write_data(Stream *stream)
{
    // shader
    if (write_segment(stream, HeaderType::SHADER, std::bind(&SceneSaveTask::write_shader, this, std::placeholders::_1)))
        return 1;

    // textures
    if (write_segment(stream, HeaderType::TEXTURE, std::bind(&SceneSaveTask::write_texture, this, std::placeholders::_1)))
        return 2;

    // material
    if (write_segment(stream, HeaderType::MATERIAL, std::bind(&SceneSaveTask::write_material, this, std::placeholders::_1)))
        return 3;

    // Mesh
    if (write_segment(stream, HeaderType::MESH, std::bind(&SceneSaveTask::write_mesh, this, std::placeholders::_1)))
        return 4;

    // Objects
    if (write_segment(stream, HeaderType::OBJECT, std::bind(&SceneSaveTask::write_gameobject, this, std::placeholders::_1)))
        return 5;

    // script
    if (write_segment(stream, HeaderType::SCRIPT, std::bind(&SceneSaveTask::write_script, this, std::placeholders::_1)))
        return 6;

    // end file
    if (write_segment(stream, HeaderType::NONE, nullptr))
        return 7;

    return 0;
}


int SceneSaveTask::write_metadata(Stream *stream)
{
    std::string name = SceneManager::GetSceneName(m_scene);

    if (!stream->write(name))
        return 1;

    read_gameobject();

    return 0;
}

int SceneSaveTask::write_shader(Stream *stream)
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

int SceneSaveTask::write_texture(Stream *stream)
{
    return 0;
}

int SceneSaveTask::write_material(Stream *stream)
{
    /*
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
    */
    return 0;
}

int SceneSaveTask::write_mesh(Stream *stream)
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

        // MemoryStream temp;
        // mesh->serialize(&temp);
        //
        // std::size_t length = temp.tellg();
        // if (!stream->write(length))
        //     return 3;
        //
        // mesh->serialize(stream);
    }
    return 0;
}

int SceneSaveTask::write_gameobject(Stream *stream)
{
    auto objectList = m_scene->getObjectList();

    auto gocount = objectList.size();
    if (!stream->write(gocount))
        return 1;

    for (auto& gameObjectID : objectList)
    {
        auto gameObject = GameObject::Get(gameObjectID);
        if (!serialize(stream, gameObject))
            return 2;
    }

    return 0;
}

int SceneSaveTask::write_script(Stream *stream)
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

bool SceneSaveTask::serialize(Stream *stream, const GameObject *go)
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
        if (!serialize(stream, ComponentStorage::GetComponent<Transform>(child)->GetGameObject()))
        {
            return false;
        }
    }
    
    return true;
}

void SceneSaveTask::read_gameobject()
{
    for (auto& gameObjectID : m_scene->getObjectList())
    {
        auto gameObject = GameObject::Get(gameObjectID);

        read_data(gameObject);
    }
}

void SceneSaveTask::read_data(const GameObject *gameObject)
{
    std::vector<std::tuple<uint64_t, const Component*>> behaviorList;
    for (uint64_t hash : gameObject->GetHashes())
        behaviorList.emplace_back(hash, gameObject->getComponent(hash));

    m_componentdata.emplace_back(gameObject->GetName(), behaviorList);

    auto meshFilter = gameObject->getComponent<MeshFilter>();
    if (meshFilter != nullptr)
        m_meshmap.emplace(meshFilter->getMeshID());

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
        read_data(ComponentStorage::GetComponent<Transform>(child)->GetGameObject());
}




