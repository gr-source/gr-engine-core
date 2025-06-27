#include "gr-base/core/task/runtime/gameobjectloadtask.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/geometry/shader.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"

#include <gr-render/gShader.h>

GameObjectLoadTask::GameObjectLoadTask(const std::string &path, std::function<void (GameObject *)> func) : m_path(path), m_func(func), m_rootNode(nullptr)
{}

// ***** private ***** //
int GameObjectLoadTask::doInBackground()
{
    Console::LogInfo("[ GameObjectLoadTask ] Loading file: %s...", m_path.c_str());

    auto stream = std::make_shared<Stream>(m_path, std::ios_base::in | std::ios_base::binary);

    FileHeader fileHeader;
    if (!stream->read(reinterpret_cast<char *>(&fileHeader), sizeof(FileHeader)))
    {
        return 1;
    }

    if (std::strcmp(fileHeader.signature, "gmdx") != 0)
    {
        Console::LogError("[ GameObjectLoadTask ] file is not gmdx: %s", m_path.c_str());
        return 2;
    }

    int result = read_data(stream.get());
    if (result != 0)
    {
        return 3;
    }
    return 0;
}

void GameObjectLoadTask::onPostExecute(int result)
{
    if (result != 0)
    {
        return;
    }
    Console::LogInfo("[ GameObjectLoadTask ] Loaded: %s", m_path.c_str());

    /*

    auto scene = Scene::GetCurrentScene();
    if (scene != nullptr)
    {
        Scene::Use(nullptr);
    }

    auto go = GameObject::Instantiate(m_rootNode);
    
    Scene::Use(scene);
    
    Console::LogInfo("[ GameObjectLoadTask ] Model loaded sucessfuly %s.", go->getName().c_str());

    if (m_func)
    {
        m_func(go);
    }
    */
}

int GameObjectLoadTask::read_data(Stream *stream)
{
    while (true)
    {
        DataSegment dataSegment;
        if (!stream->read(reinterpret_cast<char *>(&dataSegment), sizeof(DataSegment)))
        {
            break;
        }

        if (dataSegment.type == HeaderType::NONE && dataSegment.sizeData == 0)
        {
            break;
        }

        auto next = stream->tellg() + static_cast<std::streampos>(dataSegment.sizeData);
        if (dataSegment.sizeData <= 0)
        {
            stream->seekg(next, std::ios_base::beg);
            continue;
        }

        auto name = HeaderName[dataSegment.type];

        Console::LogWarning("segment: %s", HeaderName[dataSegment.type].c_str());

        int result = 0;

        switch (dataSegment.type)
        {
            /*
            case HeaderType::OBJECT:
                result = read_gameobject();
                break;
            case HeaderType::SHADER:
                result = read_shader();
                break;
            case HeaderType::SCRIPT:
                result = read_script();
                break;
            case HeaderType::MATERIAL:
                result = read_material();
                break;
            case HeaderType::TEXTURE:
                result = read_texture();
                break;
                */
            default:
                break;
        }

        if (result != 0)
        {
            Console::LogWarning("[ GameObjectLoadTask ] Segment %s do not ready", name.c_str());
        }

        if (stream->tellg() != next)
            stream->seekg(next, std::ios_base::beg);
    }
    return 0;
}

int GameObjectLoadTask::read_gameobject(Stream *stream, Transform *parent)
{
    // read name
    std::size_t gonsize = 0;
    if (!stream->read(reinterpret_cast<char *>(&gonsize), sizeof(std::size_t)))
    {
        return 1;
    }

    std::string goname(gonsize, '\0');
    if (!stream->read(goname.data(), gonsize * sizeof(char)))
    {
        return 2;
    }

    // read tag
    std::size_t gotsize = 0;
    if (!stream->read(reinterpret_cast<char *>(&gotsize), sizeof(std::size_t))) {
        return 3;
    }

    std::string gotag(gotsize, '\0');
    if (!stream->read(gotag.data(), gotsize * sizeof(char))) {
        return 4;
    }

    bool bActive = false;
    bool bEnable = false;

    // read status
    {
        if (!stream->read(reinterpret_cast<char *>(&bActive), sizeof(bool)))
        {
            return 5;
        }

        if (!stream->read(reinterpret_cast<char *>(&bEnable), sizeof(bool)))
        {
            return 6;
        }
    }

    // create

    auto gameObject = &(*m_tempGameObject.emplace_back(std::make_shared<GameObject>(goname, gotag)));
    gameObject->setActive(bActive);
    gameObject->setStatic(bEnable);

    m_tempGameObjectMap.emplace(goname, gameObject);
    
    auto transform = gameObject->getTransform();
    if (parent != nullptr) {
        transform->setParent(parent);
    } else {
        m_rootNode = gameObject;
    }

    std::size_t ccount = 0;
    if (!stream->read(reinterpret_cast<char *>(&ccount), sizeof(std::size_t))) {
        return 7;
    }
    
    int result = 0;
    for (int i=0;i<ccount;i++) {
        result = read_gameobject(stream, transform);
        if (result != 0) {
            break;
        }
    }
    transform = nullptr;

    return result;
}

int GameObjectLoadTask::read_shader(Stream *stream)
{
    std::size_t length = {};
    if (!stream->read(reinterpret_cast<char *>(&length), sizeof(std::size_t)))
    {
        return 1;
    }

    for (std::size_t i={};i<length;i++)
    {
        std::string name_material, name_shader;

        // read material
        {
            std::size_t len = {};
            if (!stream->read(reinterpret_cast<char *>(&len), sizeof(std::size_t)))
            {
                return 2;
            }

            name_material = std::string(len, '\0');
            if (!stream->read(name_material.data(), len * sizeof(char)))
            {
                return 3;
            }
        }

        // read script
        {
            std::size_t len = {};
            if (!stream->read(reinterpret_cast<char *>(&len), sizeof(std::size_t)))
            {
                return 2;
            }

            name_shader = std::string(len, '\0');
            if (!stream->read(name_shader.data(), len * sizeof(char)))
            {
                return 3;
            }
        }

        auto info = AssetsManager::LoadShader(GetShaderParamsFromName(name_shader), &GameObjectLoadTask::emplace, this, name_material);
        info->wait();
    }
    return 0;
}

int GameObjectLoadTask::read_texture(Stream *stream)
{
    std::size_t textureCount = 0;
    if (!stream->read(reinterpret_cast<char *>(&textureCount), sizeof(std::size_t)))
    {
        return 1;
    }

    for (std::size_t i=0;i<textureCount;i++)
    {
        // read game object name
        std::size_t gonsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&gonsize), sizeof(std::size_t)))
        {
            return 2;
        }

        std::string goname(gonsize, '\0');
        if (!stream->read(goname.data(), gonsize * sizeof(char)))
        {
            return 3;
        }

        // read material name
        std::size_t mnsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&mnsize), sizeof(std::size_t)))
        {
            return 4;
        }

        std::string mname(mnsize, '\0');
        if (!stream->read(mname.data(), mnsize * sizeof(char)))
        {
            return 5;
        }

        // read texture path
        std::size_t tpsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&tpsize), sizeof(std::size_t)))
        {
            return 4;
        }

        std::string tpath(tpsize, '\0');
        if (!stream->read(tpath.data(), tpsize * sizeof(char)))
        {
            return 5;
        }
    }
    return 0;
}

int GameObjectLoadTask::read_material(Stream *stream)
{
    std::size_t sname = 0;
    if (!stream->read(reinterpret_cast<char *>(&sname), sizeof(std::size_t)))
    {
        return 1;
    }

    for (std::size_t i={};i<sname;i++)
    {
        // read game object name
        std::size_t gonsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&gonsize), sizeof(std::size_t)))
        {
            return 2;
        }

        std::string goname(gonsize, '\0');
        if (!stream->read(goname.data(), gonsize * sizeof(char)))
        {
            return 3;
        }

        // read material name
        std::size_t mnsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&mnsize), sizeof(std::size_t)))
        {
            return 4;
        }

        std::string mname(mnsize, '\0');
        if (!stream->read(mname.data(), mnsize * sizeof(char)))
        {
            return 5;
        }

        // deserialize material
        std::size_t bufferSize = 0;
        if (!stream->read(reinterpret_cast<char *>(&bufferSize), sizeof(std::size_t))) {
            return 6;
        }

        auto next = stream->tellg() + static_cast<std::streampos>(bufferSize);

        auto shader_it = m_ShaderToMaterial.find(mname);
        if (shader_it == m_ShaderToMaterial.end())
        {
            stream->seekg(next, std::ios_base::beg);
            continue;
        }

        auto material = AssetsManager::FindMaterial(mname);
        if (material == nullptr)
        {
            material = AssetsManager::CreateMaterial(mname, shader_it->second);
            material->deserialize(stream);
        }

        auto it = m_tempGameObjectMap.find(goname);
        if (it != m_tempGameObjectMap.end())
        {
            auto go = it->second;
            if (go->hasComponent<MeshRender>())
            {
                auto meshRender = go->getComponent<MeshRender>();
                if (!meshRender->hasMaterial(material))
                    meshRender->addMaterial(material);
            } else if (go->hasComponent<SkinnedMeshRender>())
            {
                auto meshRender = go->getComponent<SkinnedMeshRender>();
                if (!meshRender->hasMaterial(material))
                    meshRender->addMaterial(material);
            }

        }

        if (stream->tellg() != next)
        {
            stream->seekg(next, std::ios_base::beg);
        }
    }
    return 0;
}

int GameObjectLoadTask::read_script(Stream *stream)
{
    std::size_t gonsize = 0;
    if (!stream->read(reinterpret_cast<char *>(&gonsize), sizeof(std::size_t)))
    {
        return 1;
    }

    for (std::size_t i={};i<gonsize;i++)
    {
        std::size_t gonsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&gonsize), sizeof(std::size_t)))
        {
            return 2;
        }

        std::string goname(gonsize, '\0');
        if (!stream->read(goname.data(), gonsize * sizeof(char)))
        {
            return 3;
        }

        // read material name
        std::size_t cnsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&cnsize), sizeof(std::size_t)))
        {
            return 4;
        }

        std::string scriptname(cnsize, '\0');
        if (!stream->read(scriptname.data(), cnsize * sizeof(char)))
        {
            return 5;
        }

        std::size_t data_size = 0;
        if (!stream->read(reinterpret_cast<char *>(&data_size), sizeof(std::size_t))) {
            return 6;
        }

        auto next = stream->tellg() + static_cast<std::streampos>(data_size);

        auto it = m_tempGameObjectMap.find(goname);
        if (it == m_tempGameObjectMap.end())
        {
            stream->seekg(next, std::ios_base::beg);
            continue;
        }
        
        auto entity = it->second->getEntity();

        auto component = entity->hasComponent(scriptname) ? entity->getComponent(scriptname) : entity->addComponent(scriptname);
        if (component != nullptr && data_size > 0)
        {
            component->Deserialize(stream);
        }

        if (stream->tellg() != next)
        {
            Console::LogWarning("[ SceneTask ] %s script %s read data imcompleted!", goname.c_str(), scriptname.c_str());
            
            stream->seekg(next, std::ios_base::beg);
            continue;
        }
    }

    return 0;
}

void GameObjectLoadTask::emplace(std::string name, grr::gShader *shader)
{
    std::cout << "Emplace: " << name << std::endl;
    m_ShaderToMaterial.emplace(name, shader);
}




