#include "gr-base/core/task/runtime/gameobjectsavetask.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/geometry/material.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"

GameObjectSaveTask::GameObjectSaveTask(const std::string &path, GameObject *gameObject, std::function<void (GameObject *)> func) : m_path(path), m_gameObject(gameObject), m_func(func)
{}

// private
int GameObjectSaveTask::doInBackground()
{
    auto stream = std::make_unique<Stream>(gfile::get_cache_directory() + "/" + (m_gameObject->getName() + ".gmdx"), std::ios_base::out | std::ios_base::binary);

    // Header file
    {
        FileHeader header{{'g', 'm', 'd', 'x'}, 1.0f, 0};
        if (!stream->writeS(&header, 1, false))
        {
            Console::LogError("[ GameObjectSaveTask ] Fail to write header.");
            return 1;
        }
    }

    if (write_metadata(stream.get()) != 0)
    {
        return 2;
    }

    int result = write_data(stream.get());
    if (result != 0)
    {
        return 3;
    }

    return 0;
}

void GameObjectSaveTask::onPostExecute(int result)
{
    if (result != 0)
    {
        return Console::LogError("[ GameObjectSaveTask ] Fail to save gameObject");
    }
    auto filename = m_gameObject->getName() + ".gmdx";

    auto file = gfile::get_cache_directory() + "/" + filename;

    if (!gfile::exists(file) || !gfile::exists(m_path))
        return;

    try
    {
        File::Move(file, m_path);
    } catch (const std::exception &e)
    {
        return Console::LogError("[ GameObjectSaveTask ] %s", e.what());
    }

    Console::LogInfo("[ GameObjectSaveTask ] Saved: %s", filename.c_str());

    if (m_func != nullptr)
    {
        m_func(m_gameObject);
    }
}

int GameObjectSaveTask::write_metadata(Stream *stream)
{
    read_gameobject(m_gameObject);
    return 0;
}

int GameObjectSaveTask::write_data(Stream *stream)
{
    // write shader
    {
        if (write_shader(stream) != 0)
        {
            return 1;
        }

        DataSegment dataSegment = {HeaderType::SHADER, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&dataSegment, 1, false))
        {
            return 2;
        }
        stream->flush();
    }

    // write texture
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

    // write material
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
    
    // write mesh
    {
        if (write_mesh(stream) != 0)
            return 7;

        DataSegment dataSegment = {HeaderType::MESH, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&dataSegment, 1, false))
            return 8;

        stream->flush();
    }

    // write gameobject
    {
        if (write_gameobject(stream, m_gameObject) != 0)
        {
            return 7;
        }

        DataSegment dataSegment = {HeaderType::OBJECT, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&dataSegment, 1, false))
        {
            return 8;
        }
        stream->flush();
    }

    // write script
    {
        if (write_script(stream) != 0)
        {
            return 9;
        }

        DataSegment dataSegment = {HeaderType::SCRIPT, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&dataSegment, 1, false))
        {
            return 10;
        }
        stream->flush();
    }

    // end file
    {
        DataSegment dataSegment = {HeaderType::NONE, 0};
        if (!stream->writeS(&dataSegment, 1, false))
        {
            return 11;
        }

        stream->flush();
    }
    return 0;
}

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

        auto shadername = AssetsManager::GetShaderName(material.getShader());
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

int GameObjectSaveTask::write_gameobject(Stream *stream, const GameObject *go)
{
    if (!stream->write(go->getName()))
    {
        return 1;
    }

    if (!stream->write(go->getTag()))
    {
        return 2;
    }

    if (!stream->write(go->getActive()))
    {
        return 3;
    }

    if (!stream->write(go->getStatic()))
    {
        return 4;
    }

    auto transform = go->getTransform();

    auto children = transform->getChildren();

    auto ccount = children.size();
    if (!stream->write(ccount))
    {
        return 5;
    }

    for (auto &child : children)
    {
        int result = write_gameobject(stream, child->getGameObject());
        if (result != 0)
        {
            return 6;
        }
    }

    return 0;
}

int GameObjectSaveTask::write_script(Stream *stream)
{
    auto count = m_componentdata.size();
    if (!stream->write(count))
    {
        return 1;
    }

    for (auto &&[go_name, component_name, component] : m_componentdata)
    {
        if (!stream->write(go_name))
        {
            return 2;
        }

        if (!stream->write(component_name))
        {
            return 2;
        }

        auto pool = ComponentRegistry::Find(component_name);
        if (pool == nullptr)
        {
            return 3;
        }

        std::size_t bufferlength = {};
        {
            Stream temp;
            pool->serialize((Component *)component, &temp);
            bufferlength = temp.getDataSize();
            temp.close();
        }

        if (!stream->write(bufferlength))
        {
            return 4;
        }

        pool->serialize((Component *)component, stream);
    }
    return 0;
}

void GameObjectSaveTask::read_gameobject(const GameObject *gameObject)
{
    auto entity = gameObject->getEntity();

    for (auto &&[name, component] : entity->getComponentMap())
        m_componentdata.emplace_back(gameObject->getName(), name, component);

    if (gameObject->hasComponent<MeshFilter>())
    {
        auto meshFilter = gameObject->getComponent<MeshFilter>();
        for (auto &meshID : meshFilter->getMesh())
        {
            m_meshmap.emplace(meshID);
        }
    }

    if (gameObject->hasComponent<MeshRender>() || gameObject->hasComponent<SkinnedMeshRender>())
    {
        auto baseRender = gameObject->hasComponent<MeshRender>() ? static_cast<BaseRender *>(gameObject->getComponent<MeshRender>()) : static_cast<BaseRender *>(gameObject->getComponent<SkinnedMeshRender>());
        
        for (auto &materialID : baseRender->getMaterial())
        {
            auto material_name = AssetsManager::GetMaterialName(materialID);
            auto &material = AssetsManager::GetMaterial(materialID);

            auto shader_name = AssetsManager::GetShaderName(material.getShader());

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
    {
        read_gameobject(child->getGameObject());
    }
}



