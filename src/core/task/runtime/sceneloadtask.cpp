#include "gr-base/core/task/runtime/sceneloadtask.hpp"

#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/memoryStream.hpp"
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
    FileStream stream(m_path, StreamType::INPUT);
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
        
        if (dataSegment.type == HeaderType::NONE && dataSegment.size == 0)
        {
            break;
        }
        
        auto nextPosition = stream.tellg() + static_cast<std::streampos>(dataSegment.size);
        auto headerName = HeaderName[static_cast<uint8_t>(dataSegment.type)];

        if (dataSegment.size <= 0)
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
            stream.seekg(nextPosition);
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

        auto current = Scene::GetCurrent();

        SceneManager::SetActive(scene);

        CreateMaterialFromCache();

        CreateMeshFromCache();

        for (auto &root : m_rootList)
        {
            auto gameObjectID = CreateFromCache(root.get());

            GameObject::Instantiate(gameObjectID);
            
            GameObject::DestroyImediate(gameObjectID);
        }

        SceneManager::SetActive(current);

        for (auto &func : m_funcList)
            func(scene);
    }

    AssetsManager::RemoveTask(m_path);
}

void SceneLoadTask::CreateMaterialFromCache()
{
    Console::LogInfo("[ GameObjectLoadTask ] Creating %d materials...", m_materialCacheList.size());

    for (auto &cache : m_materialCacheList)
    {
        /*
        MaterialID materialID = AssetsManager::FindMaterial(cache.name);
        if (materialID == ID_INVALID)
        {
            materialID = AssetsManager::CreateMaterial(cache.name, &cache.material);
        }
        */
    }
}

void SceneLoadTask::CreateMeshFromCache()
{
    Console::LogInfo("[ GameObjectLoadTask ] Creating %d meshes...", m_meshCacheList.size());

    for (auto &cache : m_meshCacheList)
    {
        // auto meshID = AssetsManager::FindMesh(cache.name);
        // if (meshID == ID_INVALID)
        // {
        //     meshID = AssetsManager::CreateMesh(cache.name, &cache.mesh);
        // }
    }
}

GameObjectHandle SceneLoadTask::CreateFromCache(GameObjectCache* cache)
{
    auto gameObjectID = GameObject::Create(cache->name);

    auto gameObject = GameObject::Get(gameObjectID);
    gameObject->SetIsStatic(cache->IsStatic);

    MemoryStream stream(cache->buffer.data(), cache->buffer.size());
    if (read_script_from_cache(gameObject, &stream) != 0)
    {
        Console::LogWarning("[ GameObjectLoadTask ] Adding Script falied: %s", cache->name.c_str());
    }

    auto transform = gameObject->GetTransform();

    for (auto &child : cache->chindren)
    {
        auto childID = CreateFromCache(child.get());

        auto goChild = GameObject::Get(childID);

        transform->addChildren(goChild->GetTransform());
    }

    return gameObjectID;
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
        /*
        auto shader = AssetsManager::FindShader(shadername);
        if (shader == nullptr)
        {
            auto info = AssetsManager::LoadShaderAsync(GetShaderParamsFromName(shadername));
            shader = info->get<gr::Shader*>();
        }
        if (shader != nullptr)
            m_shadermap.emplace(shadername, shader);
        */
    }
    return 0;
}

int SceneLoadTask::read_texture(Stream *stream)
{
    return 0;
}

int SceneLoadTask::read_material(Stream *stream)
{
    /*
    std::size_t length = {};
    if (!stream->read(length))
        return 1;

    for (std::size_t i=0;i<length;i++)
    {
        std::string name, shadername;
        if (!stream->read(name))
            return 2;

        if (!stream->read(shadername))
            return 3;

        // deserialize material
        std::size_t bufferSize = {};
        if (!stream->read(bufferSize))
            return 4;

        auto& cache = m_materialCacheList.emplace_back();
        cache.name = name;
        cache.material.deserialize(stream);
    }
    */
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

        // auto& cache = m_meshCacheList.emplace_back();
        // cache.name = name;
        // cache.mesh.deserialize(stream);

        /*
        auto next = stream->tellg() + static_cast<std::streampos>(buffsize);

        auto meshID = AssetsManager::FindMesh(name);
        if (meshID == ID_INVALID)
        {
            meshID = AssetsManager::CreateMesh(name);

            Mesh* mesh = AssetsManager::GetMesh(meshID);
            mesh->deserialize(stream);
        } else
        {
            stream->seekg(next);
        }
        */
    }
    return 0;
}

int SceneLoadTask::read_gameobject(Stream *stream)
{
    std::size_t count = {};
    if (!stream->read(count))
        return 1;

    for (std::size_t i={};i<count;i++)
    {
        auto gameObject = deserialize(stream);
        if (gameObject == nullptr)
            return 2;

        m_rootList.push_back(std::move(gameObject));
    }

    return 0;
}

int SceneLoadTask::read_script(Stream *stream)
{
    std::size_t count = {};
    if (!stream->read(count))
        return 1;

    for (std::size_t i=0;i<count;i++)
    {
        std::string name;
        if (!stream->read(name))
            return 2;

        uint64_t buffersize;
        if (!stream->read(buffersize))
            return 3;

        std::vector<uint8_t> buffer(buffersize, 0);
        if (!stream->read(buffer.data(), buffersize))
            return 4;

        auto cache = m_gameobjectmap.find(name);
        if (cache != m_gameobjectmap.end())
            cache->second->buffer = buffer;
    }
    return 0;
}

int SceneLoadTask::read_script_from_cache(GameObject* gameObject, Stream* stream)
{
    uint32_t behaviorCount;
    if (!stream->read(behaviorCount))
        return 1;

    for (uint32_t j=0;j<behaviorCount;j++)
    {
        uint64_t componentHash;
        if (!stream->read(componentHash))
            return 3;

        size_t bufferlength = {};
        if (!stream->read(bufferlength))
            return 4;

        auto next = stream->tellg() + static_cast<std::streampos>(bufferlength);

        auto component = gameObject->getComponent(componentHash);
        if (component == nullptr)
            component = gameObject->addComponent(componentHash);

        if (component != nullptr && bufferlength > 0)
            component->Deserialize(stream);

        if (stream->tellg() != next)
            stream->seekg(next);
    }
    return 0;
}


std::unique_ptr<SceneLoadTask::GameObjectCache> SceneLoadTask::deserialize(Stream *stream)
{
    std::string name;
    if (!stream->read(name))
        return nullptr;

    bool IsStatic = false;
    if (!stream->read(IsStatic))
        return nullptr;

    auto data = std::make_unique<GameObjectCache>();
    data->name = name;
    data->IsStatic = IsStatic;

    m_gameobjectmap.emplace(name, data.get());

    std::size_t count = {};
    if (!stream->read(count))
        return nullptr;

    for (std::size_t i={};i<count;i++)
    {
        auto result = deserialize(stream);
        if (result == nullptr)
            break;

        data->chindren.push_back(std::move(result));
    }

    return data;
}

