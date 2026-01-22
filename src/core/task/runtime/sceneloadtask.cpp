#include "gr-base/core/task/runtime/sceneloadtask.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/sceneManager.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/script/transform.hpp"

// ***** public ***** //
SceneLoadTask::SceneLoadTask(const std::string &path) : m_path(path)
{}

void SceneLoadTask::addFunction(std::function<void (Scene *)> func)
{
    if (func == nullptr)
        return;

    std::lock_guard<std::mutex> lock(mutex);
    m_funcList.push_back(func);
}

// ***** private ***** //
int SceneLoadTask::doInBackground()
{
    Stream stream(m_path, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open())
    {
        Console::LogError("[ SceneLoadTask ] Faleid to open scene: ", m_path.c_str());
        return 1;
    }

    // read header data
    {
        FileHeader header;
        if (!stream.read(reinterpret_cast<char *>(&header), sizeof(FileHeader)))
        {
            Console::LogError("[ SceneLoadTask ] Failed to read binary header: %s", m_path.c_str());
            return 2;
        }

        Console::LogInfo("[ SceneLoadTask ] loading scene version: %f", header.version);
    }

    if (read_metadata(&stream) != 0)
        return 3;

    // read segment
    while (true)
    {
        DataSegment dataSegment;
        if (!stream.read(reinterpret_cast<char *>(&dataSegment), sizeof(DataSegment)))
            return 4;
        
        if (dataSegment.type == HeaderType::NONE && dataSegment.sizeData == 0)
        {
            break;
        }
        
        auto nextPosition = stream.tellg() + static_cast<std::streampos>(dataSegment.sizeData);
        auto headerName = HeaderName[dataSegment.type];

        if (dataSegment.sizeData <= 0)
        {
            continue;
        }

        int result = 0;
        switch (dataSegment.type)
        {
            case HeaderType::SHADER:
                // if (flags & SceneLoadFlags_Shader)
                    result = read_shader(&stream);
                break;
            case HeaderType::TEXTURE:
                // if (flags & SceneLoadFlags_Texture)
                    result = read_texture(&stream);
                break;
            case HeaderType::MATERIAL:
                // if (flags & SceneLoadFlags_Material)
                    result = read_material(&stream);
                break;
            case HeaderType::MESH:
                // if (flags & SceneLoadFlags_Mesh)
                    result = read_mesh(&stream);
                break;
            case HeaderType::OBJECT:
                // if (flags & SceneLoadFlags_Object)
                    result = read_gameobject(&stream);
                break;
            case HeaderType::SCRIPT:
                // if (flags & SceneLoadFlags_Script)
                    result = read_script(&stream);
                break;
            default:
                break;
        }
 
        if (stream.tellg() != nextPosition)
        {

            if (result != 0)
            {
                Console::LogError("[ SceneLoadTask ] Falied to read segment data: %s code: %d", headerName.c_str(), result);
            } else
            {
                Console::LogWarning("[ SceneLoadTask ] Data segment: %s is not fully loaded.", headerName.c_str());
            }
            stream.seekg(nextPosition, std::ios_base::beg);
            continue;
        }
    }

    return 0;
}

void SceneLoadTask::onPostExecute(int result)
{
    if (result != 0)
    {
        Console::LogError("[ SceneLoadTask ] Fail to load scene");
    } else
    {
        Console::LogError("[ SceneLoadTask ] Scene loaded sucessfuly");

        auto scene = SceneManager::Create(m_name);

        for (auto &go : m_rootList)
        {
            GameObject::Instantiate(go);

            GameObject::DestroyImediate(go);
        }

        for (auto &func : m_funcList)
            func(scene);
    }

    AssetsManager::RemoveTask(m_path);
}

int SceneLoadTask::read_metadata(Stream *stream)
{
    if (!stream->read(m_name))
    {
        return 1;
    }

    return 0;
}


int SceneLoadTask::read_shader(Stream *stream)
{
    std::size_t length = {};
    if (!stream->read(length))
    {
        return 1;
    }

    for (std::size_t i={};i<length;i++)
    {
        std::string shadername;
        if (!stream->read(shadername))
        {
            return 2;
        }
        auto shader = AssetsManager::FindShader(shadername);
        if (shader == nullptr)
        {
            auto info = AssetsManager::LoadShaderAsync(GetShaderParamsFromName(shadername));
            shader = info->get();
        }
        if (shader != nullptr)
            m_shadermap.emplace(shadername, shader);
    }
    return 0;
}

int SceneLoadTask::read_texture(Stream *stream)
{
    return 0;
}

int SceneLoadTask::read_material(Stream *stream)
{
    std::size_t length = {};
    if (!stream->read(length))
    {
        return 1;
    }

    for (std::size_t i=0;i<length;i++)
    {
        std::string name, shadername;
        if (!stream->read(name))
        {
            return 2;
        }

        if (!stream->read(shadername))
        {
            return 3;
        }

        // deserialize material
        std::size_t bufferSize = {};
        if (!stream->read(bufferSize))
        {
            return 4;
        }

        auto next = stream->tellg() + static_cast<std::streampos>(bufferSize);

        auto it = m_materialmap.find(name);
        if (it == m_materialmap.end() && m_shadermap.find(shadername) != m_shadermap.end())
        {
            MaterialID materialID = AssetsManager::FindMaterial(name);
            if (materialID == nill)
                materialID = AssetsManager::CreateMaterial(name);
            
            {
                auto &material = AssetsManager::GetMaterial(materialID);
                material.deserialize(stream);
            }

            m_materialmap.emplace(name, materialID);
        }

        if (stream->tellg() != next)
            stream->seekg(next, std::ios_base::beg);
    }
    return 0;
}

int SceneLoadTask::read_mesh(Stream *stream)
{
    std::size_t size = {};
    if (!stream->read(size))
        return 1;

    for (std::size_t i=0;i<size;i++)
    {
        std::string name;
        if (!stream->read(name))
            return 2;

        std::size_t buffsize = 0;
        if (!stream->read(buffsize))
            return 3;

        auto next = stream->tellg() + static_cast<std::streampos>(buffsize);

        auto meshID = AssetsManager::FindMesh(name);
        if (meshID == ID_INVALID)
        {
            meshID = AssetsManager::CreateMesh(name);

            Mesh* mesh = AssetsManager::GetMesh(meshID);
            mesh->deserialize(stream);
        } else
        {
            stream->seekg(next, std::ios_base::beg);
        }
    }
    return 0;
}

int SceneLoadTask::read_gameobject(Stream *stream)
{
    std::size_t count = {};
    if (!stream->read(count))
    {
        return 1;
    }

    for (std::size_t i={};i<count;i++)
    {
        auto gameObject = deserialize(stream);
        if (gameObject == nullptr)
        {
            return 2;
        }
        m_rootList.push_back(gameObject);
    }

    return 0;
}

int SceneLoadTask::read_script(Stream *stream)
{
    std::size_t count = {};
    if (!stream->read(count))
    {
        return 1;
    }

    for (std::size_t i=0;i<count;i++)
    {
        std::string go_name;
        if (!stream->read(go_name))
        {
            return 2;
        }

        uint64_t componentHash;
        if (!stream->read(componentHash))
        {
            return 2;
        }

        std::size_t bufferlength = {};
        if (!stream->read(bufferlength))
        {
            return 4;
        }

        auto next = stream->tellg() + static_cast<std::streampos>(bufferlength);

        auto it = m_gameobjectmap.find(go_name);
        if (it != m_gameobjectmap.end())
        {
            auto gameObject = GameObject::Get(it->second);
            auto component = gameObject->getComponent(componentHash);
            if (component == nullptr)
                component = gameObject->addComponent(componentHash);

            component->Deserialize(stream);

            /*
            auto pool = ComponentManager::Find(component_name);
            if (pool != nullptr)
            {
                auto entity = it->second;

                auto component = entity->hasComponent(component_name) ? entity->getComponent(component_name) : entity->addComponent(component_name);

                pool->deserialize(component, stream);
            }
            */
        }

        if (stream->tellg() != next)
        {
            stream->seekg(next, std::ios_base::beg);
        }
    }
    return 0;
}

GameObject *SceneLoadTask::deserialize(Stream *stream, Transform *parent)
{
    std::string name /*, tag;
    bool bactive, bstatic */;

    if (!stream->read(name))
    {
        return nullptr;
    }

    /*
    if (!stream->read(tag))
    {
        return nullptr;
    }
    
    if (!stream->read(bactive))
    {
        return nullptr;
    }

    if (!stream->read(bstatic))
    {
        return nullptr;
    }
    */

    auto gameObjectID = GameObject::Create(name);

    m_gameobjectmap.emplace(name, gameObjectID);

    auto gameObject = GameObject::Get(gameObjectID);
    auto transform = gameObject->getTransform();
    if (parent != nullptr)
    {
        transform->setParent(parent);
    }

    std::size_t count = {};
    if (!stream->read(count))
    {
        return nullptr;
    }

    for (std::size_t i={};i<count;i++)
    {
        if (!deserialize(stream, transform))
        {
            return nullptr;
        }
    }

    return gameObject;
}

