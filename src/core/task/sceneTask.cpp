#include "gr-base/core/task/sceneTask.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/threadpool.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/core/event.hpp"

#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/core/task/taskStatus.hpp"

std::mutex SceneTask::m_mutex;

SceneTask::SceneTask(const std::string &path, SceneLoadFlags_ flags, SceneFunc func) : stream(nullptr), scene(nullptr), status(TaskStatus::NONE)
{
    if (func)
        function_list.push_back(func);

    Threadpool::AddTask(&SceneTask::doInBackground, this, path, flags);
}

void SceneTask::render()
{
}

void SceneTask::addFunction(SceneFunc func)
{
    function_list.push_back(func);
}

TaskStatus SceneTask::getStatus() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return status;
}

void SceneTask::setStatus(TaskStatus s)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    status = s;
}

void SceneTask::upload(std::string path)
{
    auto current = Scene::AddScene(scene_name, std::move(scene));
    current->setName(scene_name);


    auto gocamera = GameObject::Create("MainEngineCamera", "Engine");
    if (gocamera) {
        gocamera->addComponent<Camera>();
        
        Scene::Init(gocamera);
        current->setCamera(gocamera);
    }

    Scene::Use(current);

    for (auto go : tempRootNode) {
        GameObject::Instantiate(go);
    }

    invoke(current);

    AssetsManager::RemoveTask(path);
}

void SceneTask::invoke(Scene *scene)
{
    for (auto func : function_list) {
        func(scene);
    }
}

/* ********** protected ********** */
void SceneTask::doInBackground(std::string path, SceneLoadFlags_ flags)
{

    setStatus(TaskStatus::PROCESS);

    stream = std::make_shared<Stream>(path, std::ios_base::in | std::ios_base::binary);
    if (!stream->is_open())
    {
        setStatus(TaskStatus::FAIL);
        return Console::LogError("[ SceneTask ] Faleid to open scene: ", path.c_str());
    }

    // read header data
    {
        FileHeader header;
        if (!stream->read(reinterpret_cast<char *>(&header), sizeof(FileHeader)))
        {
            setStatus(TaskStatus::FAIL);
            return Console::LogError("[ SceneTask ] Failed to read binary header: %s", path.c_str());
        }

        Console::LogInfo("[ SceneTask ] loading scene version: %f", header.version);
    }

    scene = std::make_unique<Scene>("");

    // read segment
    while (true)
    {
        DataSegment data;
        if (!stream->read(reinterpret_cast<char *>(&data), sizeof(DataSegment)))
        {
            setStatus(TaskStatus::FAIL);
            return Console::LogError("[ SceneTask ] Scene file corrumped.");
        }
        
        if (data.type == gHeaderType_None && data.sizeData == 0)
        {
            break;
        }
        
        auto nextPosition = stream->tellg() + static_cast<std::streampos>(data.sizeData);
        auto block_name = struct_name[data.type];

        if (data.sizeData <= 0)
        {
            stream->seekg(nextPosition, std::ios_base::beg);
            continue;
        }

        int result = -1;
        switch (data.type)
        {
        case gHeaderType_Metadata:
            if (flags & SceneLoadFlags_Metadata)
                result = readMetadata();
            break;
        case gHeaderType_Texture:
            if (flags & SceneLoadFlags_Texture)
                result = readTexture();
            break;
        case gHeaderType_Material:
            if (flags & SceneLoadFlags_Material)
                result = readMaterial();
            break;
        case gHeaderType_Mesh:
            if (flags & SceneLoadFlags_Mesh)
                result = readMesh();
            break;
        case gHeaderType_Object:
            if (flags & SceneLoadFlags_Object)
                result = readObject();
            break;
        case gHeaderType_Script:
            if (flags & SceneLoadFlags_Script)
                result = readScript();
            break;
        default:
            break;
        }

        if (result != 0)
        {
            Console::LogError("[ SceneTask ] Falied to read segment data: %s code: %d", block_name, result);
        }
        
        if (stream->tellg() == nextPosition)
        {
            continue;
        }
        Console::LogWarning("[ SceneTask ] Data segment: %s is not fully loaded.", block_name);

        stream->seekg(nextPosition, std::ios_base::beg);
    }

    setStatus(TaskStatus::SUCESS);

    Console::LogInfo("[ SceneTask ] Scene loaded sucessfuly.");

    Event::Add(&SceneTask::upload, this, path);
}

// TODO: Falta adicionar suporte a game object antes de deserializar
int SceneTask::readMetadata()
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


    // camera = std::make_shared<GameObject>("MainEngineCamera", "Engine");
    // camera->addComponent<Camera>();

    // auto transform = camera->getTransform();
    // transform->Deserialize(stream.get());
    return 0;
}

int SceneTask::readTexture()
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
int SceneTask::readMaterial()
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

        auto material = AssetsManager::CreateMaterial(name);
        material->deserialize(stream.get());
    }
    return 0;
}

// TODO: Metodo ainda incompleto
int SceneTask::readMesh()
{
    return 0;
}

int SceneTask::readObject()
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

int SceneTask::readObjectNode(Transform *parent)
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
        tempRootNode.push_back(gameObject);
    }
    gameObject->setActive(active);
    gameObject->setEnable(enable);

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

int SceneTask::readScript()
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
        /*
        Event::Add(&Scene::SetupScript, it->second, componentName, std::ref(cv), std::ref(component), std::ref(result));

        std::unique_lock<std::mutex> lock(m_mutex);
        cv.wait(lock, [&component, &result] {
            return component || bQuit || result >= 0;
        });
        if (bQuit) {
            break;
        }
        lock.unlock();
        */

        if (data_size <= 0)
        {
            continue;
        }

        auto entity = it->second->getEntity();

        auto component = entity->hasComponent(componentName) ? entity->getComponent(componentName) : entity->addComponent(componentName);
        if (component != nullptr)
        {
            component->Deserialize(stream.get());
        }

        if (stream->tellg() != next)
        {
            Console::LogWarning("[ SceneTask ] %s script %s read data imcompleted!", goname.c_str(), componentName.c_str());
            
            stream->seekg(next, std::ios_base::beg);
            continue;
        }
    }
    return 0;
}
