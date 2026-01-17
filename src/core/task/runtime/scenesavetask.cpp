#include "gr-base/core/task/runtime/scenesavetask.hpp"

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

SceneSaveTask::SceneSaveTask(const std::string &path, const Scene *scene, std::function<void (const Scene *)> func) : m_path(path), m_scene(scene), m_func(func)
{}

// ***** private ***** //
int SceneSaveTask::doInBackground()
{
    auto name = SceneManager::GetSceneName(m_scene);

    auto file = m_path + "/" + name + ".scn";

    Console::LogInfo("[ SceneSaveTask ] Writing scene %s...", file.c_str());

    auto stream = std::make_shared<Stream>(file, std::ios_base::out | std::ios_base::binary);
    // header data
    {
        FileHeader header{{'g', 'r', 's', 'c'}, 1.0f, static_cast<std::size_t>(stream->getDataSize())};

        if (!stream->writeS(&header, 1, false))
        {
            return 1;
        }
    }

    if (write_metadata(stream.get()) != 0)
    {
        return 2;
    }
    stream->flush();

    if (write_data(stream.get()) != 0)
    {
        return 3;
    }

    return 0;
}

void SceneSaveTask::onPostExecute(int result)
{
    if (result != 0)
    {
        return Console::LogError("[ SceneSaveTask ] Fail to write scene, code: %d", result);
    }

    Console::LogInfo("[ SceneSaveTask ] Scene writed sucess fully!");
}

int SceneSaveTask::write_data(Stream *stream)
{
    // shader
    {
        if (write_shader(stream) != 0)
        {
            return 2;
        }

        DataSegment dataSegment = {HeaderType::SHADER, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&dataSegment, 1, false))
        {
            return 4;
        }

        stream->flush();
    }

    // textures
    {
        if (write_texture(stream) != 0)
        {
            return 3;
        }
    
        DataSegment dataSegment = {HeaderType::TEXTURE, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&dataSegment, 1, false))
        {
            return 4;
        }
    
        stream->flush();
    }

    // material
    {
        if (write_material(stream) != 0)
        {
            return 5;
        }
    
        DataSegment dataSegment = {HeaderType::MATERIAL, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&dataSegment, 1, false))
        {
            return 6;
        }
    
        stream->flush();
    }

    // Mesh
    {
        if (write_mesh(stream) != 0)
        {
            return 7;
        }

        DataSegment dataSegment = {HeaderType::MESH, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&dataSegment, 1, false))
        {
            return 8;
        }

        stream->flush();
    }
    
    // Objects
    {
        if (write_gameobject(stream) != 0)
        {
            return 9;
        }
        
        DataSegment dataSegment = {HeaderType::OBJECT, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&dataSegment, 1, false))
        {
            return 10;
        }

        stream->flush();
    }

    // script
    {
        if (write_script(stream) != 0)
        {
            return 11;
        }
        
        DataSegment dataSegment = {HeaderType::SCRIPT, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&dataSegment, 1, false))
        {
            return 12;
        }

        stream->flush();
    }

    // end file
    {
        DataSegment dataSegment{HeaderType::NONE, 0};
        if (!stream->writeS(&dataSegment, 1, false))
        {
            Console::LogError("[ Scene ] Failed to write scene: 1");
        }
        stream->flush();
    }

    return 0;
}


int SceneSaveTask::write_metadata(Stream *stream)
{
    std::string name = SceneManager::GetSceneName(m_scene);

    if (!stream->write(name))
    {
        return 1;
    }

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
    auto length = m_materialmap.size();
    if (!stream->write(length))
    {
        return 1;
    }

    for (auto &&[name, materialID] : m_materialmap)
    {
        auto &material = AssetsManager::GetMaterial(materialID);

        if (!stream->write(name))
        {
            return 2;
        }

        auto shadername = AssetsManager::GetShaderName(material.GetShader(false));
        if (!stream->write(shadername))
        {
            return 3;
        }

        // deserialize material
        std::size_t bufferSize = 0;
        {
            Stream temp;
            material.serialize(&temp);
            bufferSize = temp.getDataSize();

            temp.close();
        }

        if (!stream->write(bufferSize))
        {
            return 4;
        }

        material.serialize(stream);
    }
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

        auto &mesh = AssetsManager::GetMesh(meshID);

        Stream temp;
        mesh.serialize(&temp);

        std::size_t length = temp.getDataSize();
        if (!stream->write(length))
            return 3;

        mesh.serialize(stream);
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
    auto count = m_componentdata.size();
    if (!stream->write(count))
    {
        return 1;
    }

    for (auto &&[go_name, componentHash, component] : m_componentdata)
    {
        if (!stream->write(go_name))
            return 2;

        if (!stream->write(componentHash))
            return 2;

        /*
        auto pool = ComponentManager::Find(component_name);
        if (pool == nullptr)
        {
            return 3;
        }
        */

        std::size_t bufferlength = {};
        {
            Stream temp;
            component->Serialize(&temp);
            // pool->serialize((Component *)component, &temp);
            bufferlength = temp.getDataSize();
            temp.close();
        }

        if (!stream->write(bufferlength))
            return 4;

        // pool->serialize((Component *)component, stream);
        component->Serialize(stream);
    }
    return 0;
}

bool SceneSaveTask::serialize(Stream *stream, const GameObject *go)
{
    if (!stream->write(go->getName()))
    {
        return false;
    }

    /*
    if (!stream->write(go->getTag()))
    {
        return false;
    }
    
    if (!stream->write(go->getActive()))
    {
        return false;
    }

    if (!stream->write(go->getStatic()))
    {
        return false;
    }
    */

    auto transform = go->getTransform();

    auto children = transform->getChildren();

    auto ccount = children.size();
    if (!stream->write(ccount))
    {
        return false;
    }

    for (auto &child : children)
    {
        if (!serialize(stream, child->GetGameObject()))
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
    for (auto &hash : gameObject->GetHashes())
        m_componentdata.emplace_back(gameObject->getName(), hash, gameObject->getComponent(hash));

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
            auto &material = AssetsManager::GetMaterial(materialID);

            auto shader_name = AssetsManager::GetShaderName(material.GetShader(false));

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

    auto transform = gameObject->getTransform();
    for (auto &child : transform->getChildren())
        read_data(child->GetGameObject());
}




