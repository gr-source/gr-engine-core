#include "gr-base/core/scene.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/event.hpp"

#include "gr-base/script/baserender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/ecs/componentRegistry.hpp"

#include <algorithm>

std::unordered_map<std::string, std::unique_ptr<Scene>> Scene::m_scenes;

std::hash<std::string> Scene::hash;

std::condition_variable Scene::cv;

std::mutex Scene::m_mutex;

Scene *Scene::m_current = nullptr;

bool Scene::bQuit = false;

void Scene::SetCurrent(Scene *scene)
{
    m_current = scene;
}

Scene *Scene::GetCurrentScene()
{
    return m_current;
}

bool Scene::HasScene()
{
    return m_current != nullptr;
}

void Scene::Use(Scene *scene)
{
    if (m_current && m_current != scene && m_current->m_enable)
    {
        m_current->setEnable(false);
    }

    if (scene && scene != m_current && !scene->m_enable)
    {
        scene->setEnable(true);
    }
    m_current = scene;
}

void Scene::Initialize()
{
}

void Scene::Shutdown()
{
    m_scenes.clear();

    m_current = nullptr;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        bQuit = true;
    }
    cv.notify_all();
}

Scene *Scene::Create(const std::string &name)
{
    auto &&[it, added] = m_scenes.emplace(name, std::make_unique<Scene>(name));

    Scene::Use(&(*it->second));

    auto camera_engine = CreateEngineCamera();

    it->second->m_goCamera = camera_engine;
    it->second->m_camera = camera_engine->getComponent<Camera>();

    // camera for game
    {
        auto camera = it->second->CreateGameObject("MainCamera");
        if (!camera)
        {
            m_scenes.erase(it);

            Scene::Use(nullptr);
            Console::LogError("[ Scene ] Falied to create scene, Camera do not created.");
            return nullptr;
        }

        camera->addComponent<Camera>();
    }

    return &(*it->second);
}

void DestroyComponnet(Transform *transform)
{
    transform->entity->destroy();

    for (auto child : transform->getChildren())
    {
        DestroyComponnet(child);
    }
}

void Scene::DestroyScript()
{
    auto scene = GetCurrentScene();
    if (!scene)
    {
        return;
    }

    for (auto go : scene->m_tempInitObjects)
    {
        DestroyComponnet(go->getTransform());
    }
    scene->m_tempInitObjects.clear();
}

void Scene::CheckRequestReinstanceScript()
{
    // /*
    auto script = ComponentRegistry::getInstance().getScriptRequestReinstance();
    if (script.empty())
    {
        return;
    }

    for (auto &&[scenename, scene] : m_scenes)
    {
        for (auto scriptname : script)
        {
            scene->reinstanceScript(scriptname);

            ComponentRegistry::getInstance().removeScriptNeedsUpdate(scriptname);
        }
    }
    // */
}

void Scene::Remove(GameObject *gameObject)
{
    auto scene = GetCurrentScene();
    if (!scene)
    {
        return;
    }

    scene->remove(gameObject);
}

void Scene::Destoy(GameObject *gameObject)
{
    auto scene = GetCurrentScene();
    if (!scene)
    {
        return;
    }

    scene->destroy(gameObject);
}

void Scene::Init(GameObject *gameObject)
{
    auto entity = gameObject->getEntity();

    entity->init();

    BaseRender *baseRender = nullptr;

    if (entity->hasComponent<MeshRender>())
    {
        baseRender = static_cast<BaseRender *>(entity->getComponent<MeshRender>());
    } else if (entity->hasComponent<SkinnedMeshRender>())
    {
        baseRender = static_cast<BaseRender *>(entity->getComponent<SkinnedMeshRender>());
    }

    if (baseRender != nullptr)
    {
        baseRender->build();
    }

    auto transform = entity->getComponent<Transform>();

    for (auto child : transform->getChildren())
    {
        Scene::Init(child->getGameObject());
    }
}

Scene *Scene::AddScene(const std::string &name, std::unique_ptr<Scene> scene)
{
    m_scenes[name] = std::move(scene);

    return &(*m_scenes[name]);
}

Scene::Scene(const std::string &name) : m_name(name), m_enable(true), m_goCamera(nullptr), m_camera(nullptr) {}

Scene::~Scene()
{
    auto temp = m_objects;
    for (auto gameObject : temp)
    {
        GameObject::Destroy(gameObject);
    }
}

void Scene::setName(const std::string &name)
{
    m_name = name;
}

std::string Scene::getName() const
{
    return m_name;
}

#include "gr-base/core/task/saveSceneTask.hpp"
void Scene::save(const std::string &path)
{
    SaveSceneTask saveSceneTask(this);
}

GameObject *Scene::CreateGameObject(const std::string &name, const std::string &tag)
{
    auto gameObject = GameObject::Create(name, tag);
    if (!gameObject)
    {
        return nullptr;
    }

    m_objects.push_back(gameObject);

    return gameObject;
}

std::vector<GameObject *> Scene::getObjectList() const
{
    return m_objects;
}

void Scene::insert(GameObject *gameObject)
{
    auto it = std::find(m_objects.begin(), m_objects.end(), gameObject);
    if (it != m_objects.end())
    {
        return;
    }

    m_objects.push_back(gameObject);
}

void Scene::Clear()
{
    m_objects.clear();
}

GameObject *Scene::getCameraObject() const
{
    return m_goCamera;
}

void Scene::setCamera(GameObject *go)
{
    m_goCamera = go;

    m_camera = go->getComponent<Camera>();
}

Camera *Scene::getCamera() const
{
    return m_camera;
}

// ********** public ********** //
void Scene::select(GameObject *go)
{
    v_object_selected_list.push_back(go);

    u_object_has_been_selected.emplace(go);
}

void Scene::deselect(GameObject *go)
{
    v_object_selected_list.erase(std::remove(v_object_selected_list.begin(), v_object_selected_list.end(),  go), v_object_selected_list.end());

    u_object_has_been_selected.erase(go);
}

void Scene::deselect_all()
{
    v_object_selected_list.clear();
    u_object_has_been_selected.clear();
}

bool Scene::has_object_selected() const
{
    return !v_object_selected_list.empty();
}

bool Scene::object_has_been_selected(GameObject *go) const
{
    return u_object_has_been_selected.find(go) != u_object_has_been_selected.end();
}

std::vector<GameObject *> Scene::get_object_selected() const
{
    return v_object_selected_list;
}


std::unordered_map<std::string, std::unique_ptr<Scene>> &Scene::GetScenes()
{
    return m_scenes;
}

void Scene::InitializeEngineScript()
{
    auto scene = GetCurrentScene();
    if (!scene)
    {
        return;
    }

    for (auto go : scene->getObjectList())
    {
        Scene::InitializeInternalScript(go->getTransform());
    }
}

void Scene::InitializeAllScript()
{
    auto scene = Scene::GetCurrentScene();
    if (!scene)
    {
        return;
    }

    for (auto go : scene->getObjectList())
    {
        Scene::Init(go);
        scene->m_tempInitObjects.push_back(go);
    }
}

static std::array<std::string, 5> Scripts = {"MeshRender", "SkinnedMeshRender", "MeshFilter", "Light", "Camera"};

void Scene::InitializeInternalScript(Transform *transform)
{
    auto go = transform->getGameObject();

    auto entity = go->getEntity();
    for (auto script : Scripts)
    {
        // auto pool = ComponentRegistry::Find(script);

        if (entity->hasComponent(script))
        {
            auto component = entity->getComponent(script);

            component->Init();

            // pool->init(component);
            //
            BaseRender *baseRender = nullptr;

            if (script == "MeshRender")
            {
                baseRender = static_cast<BaseRender *>(component);
            } else if (script == "SkinnedMeshRender")
            {
                baseRender = static_cast<BaseRender *>(component);
            }

            if (baseRender != nullptr)
            {
                baseRender->build();
            }
        }
    }

    for (auto child : transform->getChildren())
    {
        InitializeInternalScript(child);
    }
    // */
}

GameObject *Scene::CreateEngineCamera()
{
    auto go = GameObject::Create("MainEngineCamera", "Engine");
    if (!go)
    {
        return nullptr;
    }

    auto camera = go->addComponent<Camera>();
    if (!camera)
    {
        return nullptr;
    }

    Scene::Init(go);
    return go;
}

void Scene::setEnable(bool value)
{
    for (auto &gameObject : m_objects)
    {
        setDisableEntity(gameObject->getTransform(), value);
    }
    m_enable = value;
}

void Scene::setDisableEntity(Transform *transform, bool value)
{
    transform->entity->setEnable(value);

    for (auto &child : transform->getChildren())
    {
        setDisableEntity(child, value);
    }
}

void Scene::SetupScene(Scene *scene)
{
    auto it = m_scenes.find(scene->getName());
    if (it == m_scenes.end())
        m_scenes.emplace(scene->getName(), std::unique_ptr<Scene>(scene));

    for (auto &go : scene->getObjectList())
    {
        Scene::Init(go);
    }
}

bool Scene::remove(GameObject *gameObject)
{
    m_tempInitObjects.erase(std::remove(m_tempInitObjects.begin(), m_tempInitObjects.end(), gameObject), m_tempInitObjects.end());

    m_objects.erase(std::remove(m_objects.begin(), m_objects.end(), gameObject), m_objects.end());
    return true;
}

bool Scene::destroy(GameObject *gameObject)
{
    if (!remove(gameObject))
    {
        Console::LogWarning("[ Scene ] GameObject don't has in scene.");
    }

    GameObject::Destroy(gameObject);
    return true;
}

void Scene::for_each(std::function<void(GameObject *)> func)
{
    for (auto object : m_objects)
    {
        for_each(object->getTransform(), func);
    }
}

void Scene::for_each(Transform *transform, std::function<void(GameObject *)> func)
{
    func(transform->getGameObject());

    for (auto child : transform->getChildren())
    {
        for_each(child, func);
    }
}

void Scene::reinstanceScript(const std::string &name)
{
    Scene::for_each([name](GameObject *go)
                    {
        auto entt = go->getEntity();

        if (!entt->hasComponent(name)) { return; }

        entt->destroy(name);

        entt->addComponent(name); });
}
