#include "gr-base/core/scene.hpp"

#include "gr-base/core/gameObject.hpp"

// #include "gr-base/core/scenemanager.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

#include <algorithm>

/*
void Scene::CheckRequestReinstanceScript()
{
    auto script = ComponentRegistry::GetScriptRequestReinstance();
    if (script.empty())
    {
        return;
    }

    for (auto &scene : SceneManager::GetScenes())
    {
        for (auto scriptname : script)
        {
            scene->reinstanceScript(scriptname);
        }
    }
}
*/
void Scene::Play(GameObject *gameObject)
{
    /*
    gameObject->play();

    auto transform = gameObject->getComponent<Transform>();

    for (auto child : transform->getChildren())
    {
        Scene::Play(child->getGameObject());
    }
    */
}

void Scene::Stop(GameObject *gameObject)
{
    /*
    gameObject->stop();

    auto transform = gameObject->getComponent<Transform>();

    for (auto child : transform->getChildren())
    {
        Scene::Stop(child->getGameObject());
    }
    */
}

// public
Scene::Scene() : m_enable(true), m_goCamera(nullptr), m_camera(nullptr), m_isrunning(false) {}

Scene::~Scene()
{
    for (auto gameObject : getObjectList())
    {
        GameObject::Destroy(gameObject);
    }

    if (m_goCamera != nullptr)
    {
        GameObject::Destroy(m_goCamera);
    }
}

void Scene::init()
{
    m_goCamera = CreateEngineCamera();

    m_camera = m_goCamera->getComponent<Camera>();
}

void Scene::play()
{
    m_isrunning = true;

    for (auto &go : getObjectList())
    {
        Scene::Play(go);
    }
}

void Scene::stop()
{
    m_isrunning = false;

    for (auto &go : getObjectList())
    {
        Scene::Stop(go);
    }
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

    GameObject::DestroyImediate(go);
}

bool Scene::is_running() const
{
    return m_isrunning;
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

    if (go != nullptr)
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

    GameObject::DestroyImediate(gameObject);
    return true;
}

GameObject *Scene::CreateEngineCamera()
{
    auto go = GameObject::Create("MainEngineCamera");
    if (!go)
        return nullptr;

    auto camera = go->addComponent<Camera>();
    if (!camera)
        return nullptr;

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
    transform->GetGameObject()->SetState(value);

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
    func(transform->GetGameObject());

    for (auto child : transform->getChildren())
    {
        for_each(child, func);
    }
}

void Scene::reinstanceScript(const std::string &name)
{
    Scene::for_each([name](GameObject *go)
    {
        // if (!go->hasComponent(name))
            // return;

        // go->free(name);

        // go->addComponent(name.c_str());
    });
}
