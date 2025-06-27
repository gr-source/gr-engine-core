#include "gr-base/core/task/runtime/scenesavetask.hpp"

#include "gr-base/core/scenemanager.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/ecs/gameObject.hpp"

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
    return 0;
}

int SceneSaveTask::write_texture(Stream *stream)
{
    return 0;
}

int SceneSaveTask::write_material(Stream *stream)
{
    return 0;
}

int SceneSaveTask::write_mesh(Stream *stream)
{
    return 0;
}

int SceneSaveTask::write_gameobject(Stream *stream)
{
    auto objectList = m_scene->getObjectList();

    auto gocount = objectList.size();
    if (!stream->write(gocount))
    {
        return 1;
    }

    for (auto go : objectList)
    {
        if (!serialize(stream, go))
        {
            return 2;
        }
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

bool SceneSaveTask::serialize(Stream *stream, const GameObject *go)
{
    if (!stream->write(go->getName()))
    {
        return false;
    }

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

    auto transform = go->getTransform();

    auto children = transform->getChildren();

    auto ccount = children.size();
    if (!stream->write(ccount))
    {
        return false;
    }

    for (auto &child : children)
    {
        if (!serialize(stream, child->getGameObject()))
        {
            return false;
        }
    }
    
    return true;
}

void SceneSaveTask::read_gameobject()
{
    for (auto &go : m_scene->getObjectList())
    {
        read_data(go);
    }
}

void SceneSaveTask::read_data(const GameObject *gameObject)
{
    auto entity = gameObject->getEntity();

    for (auto &&[name, component] : entity->getComponentMap())
        m_componentdata.emplace_back(gameObject->getName(), name, component);

    auto transform = gameObject->getTransform();
    for (auto &child : transform->getChildren())
    {
        read_data(child->getGameObject());
    }
}

/*
#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/script/transform.hpp"

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
        if (writeMetadata() != 0)
        {
            return 1;
        }
        
        DataSegment dataSegment = {HeaderType::METADATA, static_cast<std::size_t>(m_stream->getDataSize())};
        if (!m_stream->writeS(&dataSegment, 1, false))
        {
            return 2;
        }
    
        m_stream->flush();
    }

    // shader
    {
        if (write_shader() != 0)
        {
            return 2;
        }

        DataSegment dataSegment = {HeaderType::SHADER, static_cast<std::size_t>(m_stream->getDataSize())};
        if (!m_stream->writeS(&dataSegment, 1, false))
        {
            return 4;
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

int SceneSaveTask::write_shader()
{
    /*
    for (std::size_t i=0;i<IAssetShaderCount;i++)
    {
        for (std::size_t j=0;j<IShaderOptmize;j++)
        {
            auto shaderList = AssetsManager::GetShaderList(i, j);

            auto size = shaderList.size();
            if (!m_stream->write(&size, sizeof(std::size_t)))
            {
                return 3;
            }

            for (auto &shader : shaderList)
            {

                auto name = AssetsManager::GetShaderName(shader);

                auto len = name.length();
                if (!m_stream->write(&len, sizeof(std::size_t)))
                {
                    return 4;
                }

                if (!m_stream->write(name.c_str(), len * sizeof(char)))
                {
                    return 5;
                }
            }
        }
    }

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
*/



