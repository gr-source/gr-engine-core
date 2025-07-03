#include "gr-base/core/scene.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/scenemanager.hpp"
#include "gr-base/script/baserender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

#include "gr-base/ecs/gameObject.hpp"

std::hash<std::string> Scene::hash;

void DestroyComponnet(Transform *transform)
{
    transform->entity->destroy();

    for (auto child : transform->getChildren())
    {
        DestroyComponnet(child);
    }
}

void Scene::CheckRequestReinstanceScript()
{
    auto script = ComponentRegistry::getInstance().getScriptRequestReinstance();
    if (script.empty())
    {
        return;
    }

    for (auto &scene : SceneManager::GetScenes())
    {
        for (auto scriptname : script)
        {
            scene->reinstanceScript(scriptname);

            ComponentRegistry::getInstance().removeScriptNeedsUpdate(scriptname);
        }
    }
}

void Scene::Start(GameObject *gameObject)
{
    auto entity = gameObject->getEntity();

    entity->start();

    auto transform = entity->getComponent<Transform>();

    for (auto child : transform->getChildren())
    {
        Scene::Start(child->getGameObject());
    }
}

void Scene::Stop(GameObject *gameObject)
{
    auto entity = gameObject->getEntity();

    entity->destroy();

    auto transform = entity->getComponent<Transform>();

    for (auto child : transform->getChildren())
    {
        Scene::Stop(child->getGameObject());
    }
}

// public
Scene::Scene() : m_enable(true), m_goCamera(nullptr), m_camera(nullptr), m_isrunning(false) {}

Scene::~Scene()
{
    /*
    auto temp = m_objects;
    for (auto gameObject : temp)
    {
        GameObject::Destroy(gameObject);
    }
    */
}

void Scene::init()
{
    m_goCamera = CreateEngineCamera();

    m_camera = m_goCamera->getComponent<Camera>();
}

void Scene::play()
{
    for (auto &go : getObjectList())
    {
        Scene::Start(go);
    }

    m_isrunning = true;
}

void Scene::stop()
{
    for (auto &go : getObjectList())
    {
        Scene::Stop(go);
    }

    m_isrunning = false;
}

void Scene::create_main_camra()
{
    auto go = GameObject::Create("MainCamera_");
    go->addComponent<Camera>();

    auto MainCamera = GameObject::Instantiate(go);
    if (MainCamera)
    {
        MainCamera->setName("MainCamera");
    }

    GameObject::Destroy(go);
}

bool Scene::is_running() const
{
    return m_isrunning;
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

void Scene::remove(GameObject *gameObject)
{
    m_tempInitObjects.erase(std::remove(m_tempInitObjects.begin(), m_tempInitObjects.end(), gameObject), m_tempInitObjects.end());

    m_objects.erase(std::remove(m_objects.begin(), m_objects.end(), gameObject), m_objects.end());
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

bool Scene::destroy(GameObject *gameObject)
{
    remove(gameObject);

    GameObject::Destroy(gameObject);
    return true;
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

/*
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
*/


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
