#include "gr-base/core/task/runtime/gameobjectloadtask.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/shader.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"

#include <gr-render/gShader.h>

GameObjectLoadTask::GameObjectLoadTask(const std::string &path, std::function<void (GameObject *)> func) : m_path(path), m_func(func), m_gameObject(nullptr)
{}

// ***** private ***** //
int GameObjectLoadTask::doInBackground()
{
    Console::LogInfo("[ GameObjectLoadTask ] Loading file: %s...", m_path.c_str());

    Stream stream(m_path, std::ios_base::in | std::ios_base::binary);

    FileHeader fileHeader;
    if (!stream.read(reinterpret_cast<char *>(&fileHeader), sizeof(FileHeader)))
        return 1;

    if (strncmp(fileHeader.sign, "gmdx", 4) != 0)
    {
        Console::LogError("[ GameObjectLoadTask ] file is not gmdx: %s", m_path.c_str());
        return 2;
    }

    if (read_metadata(&stream) != 0)
        return 3;

    if (read_data(&stream) != 0)
        return 4;
    return 0;
}

void GameObjectLoadTask::onPostExecute(int result)
{
    if (result != 0)
    {
        return;
    }

    Console::LogInfo("[ GameObjectLoadTask ] Loaded: %s", m_path.c_str());

    if (m_func)
    {
        m_func(m_gameObject);
    }
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
            case HeaderType::SHADER:
                result = read_shader(stream);
                break;
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
                result = read_gameobject(stream);
                break;
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
            stream->seekg(next, std::ios_base::beg);
    }
    return 0;
}

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
            auto info = AssetsManager::LoadShader(GetShaderParamsFromName(shadername), [&shader](auto current) { shader = current; });
            info->wait();
        }
        if (shader != nullptr)
            m_shadermap.emplace(shadername, shader);
    }
    return 0;
}

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

        auto materialID = AssetsManager::FindMaterial(name);
        if (materialID == nill)
        {
            auto it = m_materialmap.find(name);
            if (it == m_materialmap.end() && m_shadermap.find(shadername) != m_shadermap.end())
            {
                materialID = AssetsManager::CreateMaterial(name, m_shadermap[shadername]);
                {
                    auto &material = AssetsManager::GetMaterial(materialID);
                    material.deserialize(stream);
                }
            }
        }

        if (materialID != nill)
            m_materialmap.emplace(name, materialID);

        if (stream->tellg() != next)
            stream->seekg(next, std::ios_base::beg);
    }
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

        auto next = stream->tellg() + static_cast<std::streampos>(buffsize);

        auto meshID = AssetsManager::FindMesh(name);
        if (meshID == nill)
        {
            meshID = AssetsManager::CreateMesh(name);

            auto &mesh = AssetsManager::GetMesh(meshID);
            mesh.deserialize(stream);
        } else
        {
            stream->seekg(next, std::ios_base::beg);
        }
    }
    return 0;
}

int GameObjectLoadTask::read_gameobject(Stream *stream, Transform *parent)
{
    std::string name/*, tag*/;
    // bool bactive, bstatic;

    if (!stream->read(name))
    {
        return 1;
    }

    /*
    if (!stream->read(tag))
    {
        return 2;
    }

    if (!stream->read(bactive))
    {
        return 3;
    }

    if (!stream->read(bstatic))
    {
        return 4;
    }
    */

    auto gameObject = GameObject::Create(name/*, tag */);
    // gameObject->setActive(bactive);
    // gameObject->setStatic(bstatic);

    m_gameobjectmap.emplace(name, gameObject);

    if (m_gameObject == nullptr)
        m_gameObject = gameObject;

    auto transform = gameObject->getTransform();
    if (parent != nullptr)
        transform->setParent(parent);

    std::size_t ccount = {};
    if (!stream->read(ccount))
    {
        return 5;
    }

    for (std::size_t i={};i<ccount;i++)
    {
        int result = read_gameobject(stream, transform);
        if (result != 0)
        {
            return 6;
        }
    }

    return 0;
}

int GameObjectLoadTask::read_script(Stream *stream)
{
    std::size_t count = {};
    if (!stream->read(count))
        return 1;

    for (std::size_t i=0;i<count;i++)
    {
        std::string go_name;
        if (!stream->read(go_name))
            return 2;

        uint64_t componentHash;
        if (!stream->read(componentHash))
            return 3;

        size_t bufferlength = {};
        if (!stream->read(bufferlength))
            return 4;

        auto next = stream->tellg() + static_cast<std::streampos>(bufferlength);

        auto it = m_gameobjectmap.find(go_name);
        if (it != m_gameobjectmap.end())
        {
            auto gameObject = it->second;
            
            auto component = gameObject->getComponent(componentHash);
            if (component == nullptr)
                component = gameObject->addComponent(componentHash);

            if (bufferlength > 0)
                component->Deserialize(stream);
        }

        if (stream->tellg() != next)
        {
            stream->seekg(next, std::ios_base::beg);
        }
    }
    return 0;
}





