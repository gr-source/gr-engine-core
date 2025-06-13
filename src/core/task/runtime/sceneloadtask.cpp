#include "gr-base/core/task/runtime/sceneloadtask.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/thread/threadpool.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/core/event.hpp"

#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

#include "gr-base/ecs/gameObject.hpp"

std::mutex SceneLoadTask::m_mutex;

SceneLoadTask::SceneLoadTask(const std::string &path, SceneLoadFlags_ flags, SceneFunc func) : stream(nullptr), scene(nullptr), status(TaskStatus::NONE), m_camera(nullptr)
{
    if (func)
        function_list.push_back(func);

    Threadpool::AddTask(&SceneLoadTask::doInBackground, this, path, flags);
}

void SceneLoadTask::render()
{
}

void SceneLoadTask::addFunction(SceneFunc func)
{
    function_list.push_back(func);
}

TaskStatus SceneLoadTask::getStatus() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return status;
}

void SceneLoadTask::setStatus(TaskStatus s)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    status = s;
}

void SceneLoadTask::upload(std::string path)
{
    auto current = Scene::AddScene(scene_name, std::move(scene));
    current->setName(scene_name);

    Scene::Use(nullptr);

    if (m_camera != nullptr)
    {
        current->setCamera(GameObject::Instantiate(m_camera.get()));
    } else
    {
        auto gocamera = GameObject::Create("MainEngineCamera", "Engine");
        if (gocamera)
        {
            gocamera->addComponent<Camera>();
            
            Scene::Init(gocamera);

            current->setCamera(gocamera);
        }
    }

    Scene::Use(current);

    for (auto go : tempRootNode)
    {
        GameObject::Instantiate(go);
    }

    invoke(current);

    AssetsManager::RemoveTask(path);
}

void SceneLoadTask::invoke(Scene *scene)
{
    for (auto func : function_list) {
        func(scene);
    }
}

/* ********** protected ********** */
void SceneLoadTask::doInBackground(std::string path, SceneLoadFlags_ flags)
{
    setStatus(TaskStatus::PROCESS);

    stream = std::make_shared<Stream>(path, std::ios_base::in | std::ios_base::binary);
    if (!stream->is_open())
    {
        setStatus(TaskStatus::FAIL);
        Console::LogError("[ SceneLoadTask ] Faleid to open scene: ", path.c_str());
        return AssetsManager::RemoveTask(path);
    }

    // read header data
    {
        FileHeader header;
        if (!stream->read(reinterpret_cast<char *>(&header), sizeof(FileHeader)))
        {
            setStatus(TaskStatus::FAIL);
            Console::LogError("[ SceneLoadTask ] Failed to read binary header: %s", path.c_str());
            return AssetsManager::RemoveTask(path);
        }

        Console::LogInfo("[ SceneLoadTask ] loading scene version: %f", header.version);
    }

    scene = std::make_unique<Scene>("");

    // read segment
    while (true)
    {
        DataSegment data;
        if (!stream->read(reinterpret_cast<char *>(&data), sizeof(DataSegment)))
        {
            setStatus(TaskStatus::FAIL);
            Console::LogError("[ SceneLoadTask ] Scene file corrumped.");
            return AssetsManager::RemoveTask(path);
        }
        
        if (data.type == HeaderType::NONE && data.sizeData == 0)
        {
            break;
        }
        
        auto nextPosition = stream->tellg() + static_cast<std::streampos>(data.sizeData);
        auto block_name = HeaderName[data.type];

        if (data.sizeData <= 0)
        {
            stream->seekg(nextPosition, std::ios_base::beg);
            continue;
        }

        int result = -1;
        try {
            switch (data.type)
            {
                case HeaderType::METADATA:
                    if (flags & SceneLoadFlags_Metadata)
                        result = readMetadata();
                    break;
                case HeaderType::SHADER:
                    if (flags & SceneLoadFlags_Shader)
                        result = read_shader();
                    break;
                case HeaderType::TEXTURE:
                    if (flags & SceneLoadFlags_Texture)
                        result = readTexture();
                    break;
                case HeaderType::MATERIAL:
                    if (flags & SceneLoadFlags_Material)
                        result = readMaterial();
                    break;
                case HeaderType::MESH:
                    if (flags & SceneLoadFlags_Mesh)
                        result = readMesh();
                    break;
                case HeaderType::OBJECT:
                    if (flags & SceneLoadFlags_Object)
                        result = readObject();
                    break;
                case HeaderType::SCRIPT:
                    if (flags & SceneLoadFlags_Script)
                        result = readScript();
                    break;
                default:
                    break;
            }
        } catch (const std::exception &e)
        {
            Console::LogError("[ SceneLoadTask ] %s - %s", block_name, e.what());
        }

        if (result != 0)
        {
            Console::LogError("[ SceneLoadTask ] Falied to read segment data: %s code: %d", block_name, result);
        }
        
        if (stream->tellg() == nextPosition)
        {
            continue;
        }
        Console::LogWarning("[ SceneLoadTask ] Data segment: %s is not fully loaded.", block_name);

        stream->seekg(nextPosition, std::ios_base::beg);
    }

    setStatus(TaskStatus::SUCESS);

    Console::LogInfo("[ SceneLoadTask ] Scene loaded sucessfuly.");

    Event::Add(&SceneLoadTask::upload, this, path);
}

// TODO: Falta adicionar suporte a game object antes de deserializar
int SceneLoadTask::readMetadata()
{
    std::size_t snsize = 0;
    if (!stream->read(reinterpret_cast<char *>(&snsize), sizeof(std::size_t))) {
        return 1;
    }

    std::string name(snsize, '\0');
    if (!stream->read(name.data(), snsize * sizeof(char))) {
        return 2;
    }

    scene_name = name;

    if (readObjectNode(nullptr, true) != 0)
    {
        return 3;
    }

    if (tempGameObject.empty())
    {
        return 4;
    }

    if (readScript() != 0)
    {
        return 5;
    }

    m_camera = std::move(tempGameObject.back());

    tempGameObject.pop_back();

    return 0;
}

int SceneLoadTask::read_shader()
{
    for (std::size_t i=0;i<IAssetShaderCount;i++)
    {
        for (std::size_t j=0;j<IShaderOptmize;j++)
        {
            std::size_t size = {};
            if (!stream->read(reinterpret_cast<char *>(&size), sizeof(std::size_t)))
            {
                return 3;
            }

            for (std::size_t i=0;i<size;i++)
            {
                std::size_t len = {};
                if (!stream->read(reinterpret_cast<char *>(&len), sizeof(std::size_t)))
                {
                    return 4;
                }

                std::string name(len, '\0');
                if (!stream->read(name.data(), len * sizeof(char)))
                {
                    return 5;
                }

                AssetsManager::LoadShader(j, name);
            }
        }
    }

    return 0;
}

int SceneLoadTask::readTexture()
{
    std::size_t tcount = 0;
    if (!stream->read(reinterpret_cast<char *>(&tcount), sizeof(std::size_t)))
    {
        return 1;
    }

    for (std::size_t i={};i<tcount;i++)
    {
        std::size_t psize = 0;
        if (!stream->read(reinterpret_cast<char *>(&psize), sizeof(std::size_t)))
        {
            return 2;
        }

        std::string path(psize, '\0');
        if (!stream->read(path.data(), psize * sizeof(char)))
        {
            return 3;
        }
        AssetsManager::LoadTexture(path);
    }
    return 0;
}

// TODO: Falta ter o material se caso ela já tenha sido carregado antes
int SceneLoadTask::readMaterial()
{
    std::size_t mcount = 0;
    if (!stream->read(reinterpret_cast<char *>(&mcount), sizeof(std::size_t)))
    {
        return 1;
    }

    for (std::size_t i={};i<mcount;i++)
    {
        std::size_t nsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t)))
        {
            return 2;
        }

        std::string name(nsize, '\0');
        if (!stream->read(name.data(), nsize * sizeof(char)))
        {
            return 3;
        }

        std::size_t stream_size = {};
        if (!stream->read(reinterpret_cast<char *>(&stream_size), sizeof(std::size_t)))
        {
            return 4;
        }

        auto next = stream->tellg() + static_cast<std::streampos>(stream_size);

        auto material = AssetsManager::FindMaterial(name);
        if (material == nullptr)
        {
            material = AssetsManager::CreateMaterial(name, nullptr);
            material->deserialize(stream.get());
        }

        if (stream->tellg() != next)
        {
            stream->seekg(next, std::ios_base::beg);
        }
    }
    return 0;
}

// TODO: Metodo ainda incompleto
int SceneLoadTask::readMesh()
{
    return 0;
}

int SceneLoadTask::readObject()
{
    std::size_t gocount = 0;
    if (!stream->read(reinterpret_cast<char *>(&gocount), sizeof(std::size_t)))
    {
        return 1;
    }

    int result = 0;
    for (std::size_t i={};i<gocount;i++)
    {
        result = readObjectNode();
        if (result != 0)
        {
            break;
        }
    }

    return result;
}

int SceneLoadTask::readObjectNode(Transform *parent, bool main)
{
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

    std::size_t gotsize = 0;
    if (!stream->read(reinterpret_cast<char *>(&gotsize), sizeof(std::size_t)))
    {
        return 3;
    }

    std::string gotag(gotsize, '\0');
    if (!stream->read(gotag.data(), gotsize * sizeof(char)))
    {
        return 4;
    }

    bool active = true;
    if (!stream->read(reinterpret_cast<char *>(&active), sizeof(bool)))
    {
        return 5;
    }

    bool enable = true;
    if (!stream->read(reinterpret_cast<char *>(&enable), sizeof(bool)))
    {
        return 6;
    }

    auto gameObject = &(*tempGameObject.emplace_back(std::make_shared<GameObject>(goname, gotag)));
    if (parent != nullptr) {
        gameObject->getTransform()->setParent(parent);
    } else {
        if (!main)
            tempRootNode.push_back(gameObject);
    }
    gameObject->setActive(active);
    gameObject->setStatic(enable);

    tempGameObjectMap[goname] = gameObject;

    std::size_t ccount = 0;
    if (!stream->read(reinterpret_cast<char *>(&ccount), sizeof(std::size_t)))
    {
        return 8;
    }

    int result = 0;
    for (std::size_t i=0;i<ccount;i++)
    {
        result = readObjectNode(gameObject->getTransform());
        if (result != 0)
        {
            break;
        }
    }

    gameObject = nullptr;

    return result;
}

int SceneLoadTask::readScript()
{
    std::size_t scount = 0;
    if (!stream->read(reinterpret_cast<char *>(&scount), sizeof(std::size_t)))
    {
        return 1;
    }

    for (std::size_t i=0;i<scount;i++) {
        std::size_t nsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t)))
        {
            return 2;
        }

        std::string goname(nsize, '\0');
        if (!stream->read(goname.data(), nsize * sizeof(char)))
        {
            return 3;
        }

        // ComponentName
        std::size_t cnsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&cnsize), sizeof(std::size_t)))
        {
            return 4;
        }
        
        std::string componentName(cnsize, '\0');
        if (!stream->read(componentName.data(), cnsize * sizeof(char)))
        {
            return 5;
        }

        std::size_t data_size = 0;
        if (!stream->read(reinterpret_cast<char *>(&data_size), sizeof(std::size_t)))
        {
            return 6;
        }

        auto next = stream->tellg() + static_cast<std::streampos>(data_size);
        
        auto it = tempGameObjectMap.find(goname);
        if (it == tempGameObjectMap.end())
        {
            stream->seekg(next, std::ios_base::beg);
            continue;
        }
        auto entity = it->second->getEntity();
        
        auto component = entity->hasComponent(componentName) ? entity->getComponent(componentName) : entity->addComponent(componentName);
        if (data_size > 0 && component != nullptr)
        {
            component->Deserialize(stream.get());
        }

        if (stream->tellg() != next)
        {
            Console::LogWarning("[ SceneLoadTask ] %s script %s read data imcompleted!", goname.c_str(), componentName.c_str());
            
            stream->seekg(next, std::ios_base::beg);
        }
    }
    return 0;
}
