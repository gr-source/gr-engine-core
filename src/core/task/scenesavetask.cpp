#include "gr-base/core/task/scenesavetask.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/script/transform.hpp"

#include <filesystem>

SceneSaveTask::SceneSaveTask(const Scene *scene) : m_scene(scene), m_stream(nullptr)
{
    auto filename = scene->getName() + ".scn";

    auto temp_file = std::filesystem::temp_directory_path() / filename;

    doInBackground(temp_file);

    auto to_scene_file = File::GetAsset("/scene/" + filename);
    if (File::Exists(to_scene_file))
    {
        File::Delete(to_scene_file);
    }

    File::Copy(temp_file, to_scene_file);
}

void SceneSaveTask::doInBackground(std::string path)
{
    m_stream = std::make_shared<Stream>(path, std::ios_base::out | std::ios_base::binary);

    // header data
    {
        FileHeader header{{'g', 'r', 's', 'c'}, 1.0f, static_cast<std::size_t>(m_stream->getDataSize())};

        if (!m_stream->writeS(&header, 1, false))
        {
            return Console::LogError("[ Scene ] Failed to write scene: 0");
        }
    }

    auto err = writeData();
    if (err != 0)
    {
        return Console::LogError("[ SCENE ] Failed to write scene segment: %d", err);
    }

    auto name = m_scene->getName();

    m_stream->close();
    Console::LogInfo("[ Scene ] Scene saved: %s", name.c_str());
}

int SceneSaveTask::writeData()
{

    // metadata
    {
        if (writeMetadata() != 0) {
            return 1;
        }
        
        DataSegment dataSegment = {HeaderType::METADATA, static_cast<std::size_t>(m_stream->getDataSize())};
        if (!m_stream->writeS(&dataSegment, 1, false)) {
            return 2;
        }
    
        m_stream->flush();
    }

    // textures
    {
        if (writeTexture() != 0) {
            return 3;
        }
    
        DataSegment dataSegment = {HeaderType::TEXTURE, static_cast<std::size_t>(m_stream->getDataSize())};
        if (!m_stream->writeS(&dataSegment, 1, false)) {
            return 4;
        }
    
        m_stream->flush();
    }

    // material
    {
        if (writeMaterial() != 0) {
            return 5;
        }
    
        DataSegment dataSegment = {HeaderType::MATERIAL, static_cast<std::size_t>(m_stream->getDataSize())};
        if (!m_stream->writeS(&dataSegment, 1, false)) {
            return 6;
        }
    
        m_stream->flush();
    }

    // Mesh
    {
        if (writeMesh() != 0) {
            return 7;
        }

        DataSegment dataSegment = {HeaderType::MESH, static_cast<std::size_t>(m_stream->getDataSize())};
        if (!m_stream->writeS(&dataSegment, 1, false)) {
            return 8;
        }

        m_stream->flush();
    }
    
    // Objects
    {
        if (writeObject() != 0) {
            return 9;
        }
        
        DataSegment dataSegment = {HeaderType::OBJECT, static_cast<std::size_t>(m_stream->getDataSize())};
        if (!m_stream->writeS(&dataSegment, 1, false)) {
            return 10;
        }

        m_stream->flush();
    }

    // script
    {
        if (writeScript() != 0) {
            return 11;
        }
        
        DataSegment dataSegment = {HeaderType::SCRIPT, static_cast<std::size_t>(m_stream->getDataSize())};
        if (!m_stream->writeS(&dataSegment, 1, false)) {
            return 12;
        }

        m_stream->flush();
    }

    // end file
    {
        DataSegment dataSegment{HeaderType::NONE, 0};
        if (!m_stream->writeS(&dataSegment, 1, false))
        {
            Console::LogError("[ Scene ] Failed to write scene: 1");
        }
        m_stream->flush();
    }

    return 0;
}

int SceneSaveTask::writeMetadata()
{
    auto name = m_scene->getName();

    auto snsize = name.size();
    if (!m_stream->write(&snsize, sizeof(std::size_t)))
    {
        return 1;
    }

    if (!m_stream->write(name.c_str(), snsize * sizeof(char)))
    {
        return 2;
    }

    if (writeGameObject(m_scene->getCameraObject()) != 0)
    {
        return 3;
    }

    if (writeScript() != 0)
    {
        return 4;
    }
    m_scriptCache.clear();
    
    return 0;
}

int SceneSaveTask::writeTexture()
{
    auto textureMap = AssetsManager::GetTextureRefMap();

    auto tcount = AssetsManager::GetTextureCount();
    if (!m_stream->write(&tcount, sizeof(std::size_t)))
    {
        return 2;
    }

    for (auto &&[texture, path] : textureMap)
    {
        std::size_t psize = path.size();
        if (!m_stream->write(&psize, sizeof(std::size_t)))
        {
            return 3;
        }

        if (!m_stream->write(path.c_str(), psize * sizeof(char)))
        {
            return 4;
        }
    }

    return 0;
}

int SceneSaveTask::writeMaterial()
{
    auto materialMap = AssetsManager::GetMaterialRefMap();

    auto mcount = materialMap.size();
    if (!m_stream->write(&mcount, sizeof(std::size_t)))
    {
        return 1;
    }

    for (auto &&[material, name] : materialMap)
    {
        std::size_t nsize = name.size();
        if (!m_stream->write(&nsize, sizeof(std::size_t)))
        {
            return 2;
        }

        if (!m_stream->write(name.c_str(), nsize * sizeof(char)))
        {
            return 3;
        }

        std::size_t stream_size = 0;
        {
            Stream stream;
            material->serialize(&stream);
            stream_size = stream.getDataSize();
            stream.close();
        }

        if (!m_stream->write(&stream_size, sizeof(std::size_t)))
        {
            return 4;
        }

        material->serialize(m_stream.get());
    }

    return 0;
}

int SceneSaveTask::writeMesh()
{
    return 0;
}

int SceneSaveTask::writeObject()
{
    auto objectList = m_scene->getObjectList();

    auto gocount = objectList.size();
    if (!m_stream->write(&gocount, sizeof(std::size_t))) {
        return 1;
    }

    int result = 0;
    for (auto go : objectList)
    {
        result = writeGameObject(go);
        if (result != 0)
        {
            break;;
        }
    }
    return result;
}

int SceneSaveTask::writeScript()
{
    auto size = m_scriptCache.size();

    if (!m_stream->write(&size, sizeof(std::size_t)))
    {
        return 1;
    }

    for (auto &&[objectName, componentName, component, pool] : m_scriptCache)
    {
        // ObjectName
        std::size_t gonsize = objectName.size();
        if (!m_stream->write(&gonsize, sizeof(std::size_t)))
        {
            return 2;
        }

        if (!m_stream->write(objectName.c_str(), gonsize * sizeof(char)))
        {
            return 3;
        }

        // ComponentName
        std::size_t cnsize = componentName.size();
        if (!m_stream->write(&cnsize, sizeof(std::size_t)))
        {
            return 4;
        }

        if (!m_stream->write(componentName.c_str(), cnsize * sizeof(char)))
        {
            return 5;
        }

        Stream temp;
        pool->serialize(component, &temp);

        std::size_t data_size = temp.getDataSize();
        if (!m_stream->write(&data_size, sizeof(std::size_t)))
        {
            return 6;
        }
        pool->serialize(component, m_stream.get());

        temp.close();
    }
    return 0;
}

int SceneSaveTask::writeGameObject(const GameObject *go)
{
    // write name
    {
        auto goname = go->getName();
        auto gonsize = goname.size();

        if (!m_stream->write(&gonsize, sizeof(std::size_t)))
        {
            return 1;
        }

        if (!m_stream->write(goname.c_str(), gonsize * sizeof(char)))
        {
            return 2;
        }
    }

    // write tag
    {
        auto gotag = go->getTag();
        auto gotsize = gotag.size();

        if (!m_stream->write(&gotsize, sizeof(std::size_t)))
        {
            return 3;
        }
        
        if (!m_stream->write(gotag.c_str(), gotsize * sizeof(char)))
        {
            return 4;
        }
    }

    // write status object
    {
        auto active = go->getActive();
        if (!m_stream->write(&active,  sizeof(bool)))
        {
            return 5;
        }

        auto enable = go->getStatic();
        if (!m_stream->write(&enable, sizeof(bool)))
        {
            return 6;
        }
    }

    auto entity = go->getEntity();
    {
        // Write Component

        std::list<std::tuple<std::string, Component *>> m_list;
        for (auto [name, component] : entity->getComponentMap()) {
            m_list.emplace_front(name, component);
        }

        for (auto &&[name, component] : m_list) {
            auto pool = ComponentRegistry::Find(name);
            if (!pool)
            {
                continue;
            }
            m_scriptCache.emplace_back(go->getName(), name, component, pool);
        }

    }
    auto transform = go->getTransform();

    auto children = transform->getChildren();

    auto ccount = children.size();
    if (!m_stream->write(&ccount, sizeof(std::size_t)))
    {
        return 7;
    }

    int result = 0;
    for (auto &child : children)
    {
        result = writeGameObject(child->getGameObject());
        if (result != 0)
        {
            break;
        }
    }
    
    return result;
}




