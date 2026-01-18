#include "gr-base/core/task/runtime/gameobjectsavetask.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/geometry/material.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"

GameObjectSaveTask::GameObjectSaveTask(const std::string &path, GameObjectID gameObjectID, std::function<void (GameObjectID)> func) : m_path(path), m_gameObjectID(gameObjectID), m_func(func)
{}

// private
int GameObjectSaveTask::doInBackground()
{
    auto gameObject = GameObject::Get(m_gameObjectID);

    Stream stream(gfile::get_cache_directory() + "/" + (gameObject->getName() + ".gmdx"), std::ios_base::out | std::ios_base::binary);

    // Header file
    {
        FileHeader header{{'g', 'm', 'd', 'x'}, 1.1f, 0};
        if (!stream.writeS(&header, 1, false))
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

    return 0;
}

void GameObjectSaveTask::onPostExecute(int result)
{
    if (result != 0)
    {
        return Console::LogError("[ GameObjectSaveTask ] Fail to save gameObject");
    }

    auto gameObject = GameObject::Get(m_gameObjectID);

    auto filename = gameObject->getName() + ".gmdx";
    {
        auto file = m_path + "/" + filename;
        if (gfile::exists(file))
            gfile::remove(file);
    }

    auto file = gfile::get_cache_directory() + "/" + filename;

    if (!gfile::exists(file) || !gfile::exists(m_path))
        return;

    try
    {
        gfile::move(file.c_str(), m_path.c_str());
    } catch (const std::exception &e)
    {
        return Console::LogError("[ GameObjectSaveTask ] %s", e.what());
    }

    Console::LogInfo("[ GameObjectSaveTask ] Saved: %s", filename.c_str());

    if (m_func != nullptr)
        m_func(m_gameObjectID);
}

int GameObjectSaveTask::write_metadata(Stream *stream)
{
    read_gameobject(GameObject::Get(m_gameObjectID));
    return 0;
}

int GameObjectSaveTask::write_data(Stream *stream)
{
    /*
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
    */

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
        if (write_gameobject(stream, GameObject::Get(m_gameObjectID)) != 0)
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
        auto &material = AssetsManager::GetMaterial(materialID);

        if (!stream->write(name))
            return 2;

        // serialize material
        std::size_t bufferSize = 0;
        {
            Stream temp;
            material.serialize(&temp);
            bufferSize = temp.getDataSize();

            temp.close();
        }

        if (!stream->write(bufferSize))
            return 4;

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

        Mesh* mesh = AssetsManager::GetMesh(meshID);

        Stream temp;
        mesh->serialize(&temp);

        std::size_t length = temp.getDataSize();
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

    auto transform = go->getTransform();

    auto children = transform->getChildren();

    auto ccount = children.size();
    if (!stream->write(ccount))
        return 5;

    for (auto &child : children)
    {
        int result = write_gameobject(stream, child->GetGameObject());
        if (result != 0)
            return 6;
    }

    return 0;
}

int GameObjectSaveTask::write_script(Stream *stream)
{
    auto count = m_componentdata.size();
    if (!stream->write(count))
        return 1;

    for (auto &&[go_name, componentHash, component] : m_componentdata)
    {
        if (!stream->write(go_name))
            return 2;

        if (!stream->write(componentHash))
            return 3;

        size_t bufferlength = {};
        {
            Stream temp;
            component->Serialize(&temp);
            bufferlength = temp.getDataSize();
            temp.close();
        }

        if (!stream->write(bufferlength))
            return 4;

        if (bufferlength > 0)
            component->Serialize(stream);
    }
    return 0;
}

void GameObjectSaveTask::read_gameobject(const GameObject *gameObject)
{
    for (uint64_t hash : gameObject->GetHashes())
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

            {
                auto it = m_materialmap.find(material_name);
                if (it == m_materialmap.end())
                    m_materialmap.emplace(material_name, materialID);
            }
        }
    }

    auto transform = gameObject->getTransform();
    for (auto &child : transform->getChildren())
        read_gameobject(child->GetGameObject());
}



