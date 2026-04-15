#include "gr-base/core/task/runtime/gameobjectloadtask.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/memoryStream.hpp"
#include "gr-base/core/file/stream.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/geometry/default.hpp"
// #include "gr-base/core/geometry/shader.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"

#include <gr-render/shader.hpp>

GameObjectLoadTask::GameObjectLoadTask(const std::string &path, std::function<void (GameObjectHandle)> func) : m_path(path), m_func(func), m_gameObject(HANDLE_INVALID), m_root(nullptr)
{}

// ***** private ***** //
int GameObjectLoadTask::doInBackground()
{
    Console::LogInfo("[ GameObjectLoadTask ] Loading file: %s...", m_path.c_str());

    FileStream stream(m_path, StreamType::INPUT);

    FileHeader fileHeader;
    if (!stream.read(&fileHeader, sizeof(FileHeader)))
        return 1;

    if (strncmp(fileHeader.sign, "gmdx", 4) != 0)
    {
        Console::LogError("[ GameObjectLoadTask ] file is not gmdx: %s", m_path.c_str());
        return 2;
    }

    if (read_metadata(&stream) != 0)
        return 3;

    if (read_data(&stream) != 0)
        return 5;

    return 0;
}

void GameObjectLoadTask::onPostExecute(int result)
{
    if (result != 0)
        return Console::LogError("[ GameObjectLoadTask ] fail to load: %s", m_path.c_str());

    CreateMaterialFromCache();

    CreateMeshFromCache();

    auto gameObjectID = CreateFromCache(m_root.get());

    Console::LogInfo("[ GameObjectLoadTask ] Loaded: %s", m_path.c_str());

    if (m_func)
        m_func(gameObjectID);
}

void GameObjectLoadTask::CreateMaterialFromCache()
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

void GameObjectLoadTask::CreateMeshFromCache()
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

GameObjectHandle GameObjectLoadTask::CreateFromCache(GameObjectCache* cache)
{
    auto gameObjectID = GameObject::Create(cache->name);

    auto gameObject = GameObject::Get(gameObjectID);
    gameObject->SetIsStatic(cache->IsStatic);
    gameObject->SetIsPrefab(true);

    MemoryStream stream(cache->buffer.data(), cache->buffer.size());
    if (read_script_from_cache(gameObject, &stream) != 0)
        Console::LogWarning("[ GameObjectLoadTask ] Adding Script falied: %s", cache->name.c_str());

    for (auto &child : cache->chindren)
    {
        auto childID = CreateFromCache(child.get());

        auto goChild = GameObject::Get(childID);

        gameObject = GameObject::Get(gameObjectID);

        auto transform = gameObject->GetTransform();
        transform->addChildren(goChild->GetTransform());
    }

    return gameObjectID;
}


int GameObjectLoadTask::read_metadata(Stream *stream)
{
    return 0;
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

        if (dataSegment.type == HeaderType::NONE && dataSegment.size == 0)
        {
            break;
        }

        auto next = stream->tellg() + static_cast<std::streampos>(dataSegment.size);
        if (dataSegment.size <= 0)
        {
            stream->seekg(next);
            continue;
        }

        auto name = HeaderName[static_cast<uint8_t>(dataSegment.type)];

        Console::LogWarning("segment: %s", name.c_str());

        int result = 0;

        switch (dataSegment.type)
        {
            case HeaderType::TEXTURE:
                result = read_texture(stream);
                break;
            case HeaderType::MATERIAL:
                result = read_material(stream);
                break;
            case HeaderType::MESH:
                result = read_mesh(stream);
                break;
            case HeaderType::OBJECT:
                {
                    auto root = read_gameobject(stream);
                    if (m_root == nullptr)
                        m_root = std::move(root);
                    break;
                }
            case HeaderType::SCRIPT:
                result = read_script(stream);
                break;
            default:
                break;
        }

        if (result != 0)
        {
            Console::LogWarning("[ GameObjectLoadTask ] Segment %s do not ready", name.c_str());
        }

        if (stream->tellg() != next)
            stream->seekg(next);
    }
    return 0;
}

/*
int GameObjectLoadTask::read_shader(Stream *stream)
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
*/

int GameObjectLoadTask::read_texture(Stream *stream)
{
    /*
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
    */
    return 0;
}

int GameObjectLoadTask::read_material(Stream *stream)
{
    /*
    std::size_t length = {};
    if (!stream->read(length))
        return 1;

    for (std::size_t i=0;i<length;i++)
    {
        std::string name;
        if (!stream->read(name))
            return 2;

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

int GameObjectLoadTask::read_mesh(Stream *stream)
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

std::unique_ptr<GameObjectLoadTask::GameObjectCache> GameObjectLoadTask::read_gameobject(Stream *stream)
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

    std::size_t ccount = {};
    if (!stream->read(ccount))
        return nullptr;

    for (std::size_t i={};i<ccount;i++)
    {
        auto result = read_gameobject(stream);
        if (result == nullptr)
            break;

        data->chindren.push_back(std::move(result));
    }

    return data;
}

int GameObjectLoadTask::read_script(Stream *stream)
{
    std::size_t gocount = {};
    if (!stream->read(gocount))
        return 1;

    for (std::size_t i=0;i<gocount;i++)
    {
        std::string name;
        if (!stream->read(name))
            return 2;

        uint64_t buffersize;
        if (!stream->read(buffersize))
            return 4;

        std::vector<uint8_t> buffer(buffersize, 0);
        if (!stream->read(buffer.data(), buffersize))
            return 5;

        auto cache = m_gameobjectmap.find(name);
        if (cache != m_gameobjectmap.end())
            cache->second->buffer = buffer;
    }
    return 0;
}

int GameObjectLoadTask::read_script_from_cache(GameObject* gameObject, Stream* stream)
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





