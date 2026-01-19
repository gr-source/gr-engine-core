#include "gr-base/core/scene.hpp"

#include "gr-base/core/default.hpp"
#include "gr-base/core/gameObject.hpp"

// #include "gr-base/core/scenemanager.hpp"
#include "gr-base/managers/gameobjectmanager.hpp"
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
Scene::Scene() : m_enable(true), m_goCamera(GID_INVALID), m_camera(nullptr), m_isrunning(false) {}

Scene::~Scene()
{
    for (auto gameObject : getObjectList())
    {
        GameObject::Destroy(gameObject);
    }

    if (m_goCamera != GID_INVALID)
    {
        GameObject::DestroyImediate(GameObject::Get(m_goCamera));
    }
}

void Scene::init()
{
    m_goCamera = CreateEngineCamera();

    auto gameObject = GameObject::Get(m_goCamera);

    m_camera = gameObject->getComponent<Camera>();
}

void Scene::play()
{
    /*
    m_isrunning = true;

    for (auto &go : getObjectList())
    {
        Scene::Play(go);
    }
    */
}

void Scene::stop()
{
    /*
    m_isrunning = false;

    for (auto &go : getObjectList())
    {
        Scene::Stop(go);
    }
    */
}

void Scene::create_main_camra()
{
    auto gameObjectID = GameObject::Create("MainCamera");

    auto gameObject = GameObjectManager::Get(gameObjectID);
    gameObject->addComponent<Camera>();

    insert(gameObjectID);
}

bool Scene::is_running() const
{
    return m_isrunning;
}

std::vector<GameObjectID> Scene::getObjectList() const
{
    return m_objects;
}

void Scene::insert(GameObjectID gameObjectID)
{
    auto it = std::find(m_objects.begin(), m_objects.end(), gameObjectID);
    if (it != m_objects.end())
    {
        return;
    }

    m_objects.push_back(gameObjectID);
}

void Scene::remove(GameObjectID gameObjectID)
{
    // m_tempInitObjects.erase(std::remove(m_tempInitObjects.begin(), m_tempInitObjects.end(), gameObjectID), m_tempInitObjects.end());

    m_objects.erase(std::remove(m_objects.begin(), m_objects.end(), gameObjectID), m_objects.end());
}

void Scene::Clear()
{
    m_objects.clear();
}

GameObjectID Scene::getCameraObject() const
{
    return m_goCamera;
}

void Scene::setCamera(GameObjectID id)
{
    m_goCamera = id;

    if (id != GID_INVALID)
    {
        // m_camera = go->getComponent<Camera>();
    }
}

Camera *Scene::getCamera() const
{
    return m_camera;
}

// ********** public ********** //
void Scene::select(GameObjectID id)
{
    v_object_selected_list.push_back(id);

    u_object_has_been_selected.emplace(id);
}

void Scene::deselect(GameObjectID id)
{
    v_object_selected_list.erase(std::remove(v_object_selected_list.begin(), v_object_selected_list.end(), id), v_object_selected_list.end());

    u_object_has_been_selected.erase(id);
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

bool Scene::object_has_been_selected(GameObjectID id) const
{
    return u_object_has_been_selected.find(id) != u_object_has_been_selected.end();
}

std::vector<GameObjectID> Scene::get_object_selected() const
{
    return v_object_selected_list;
}

bool Scene::destroy(GameObjectID gameObjectID)
{
    remove(gameObjectID);

    GameObject::DestroyImediate(gameObjectID);
    return true;
}

GameObjectID Scene::CreateEngineCamera()
{
    auto gameObjectID = GameObject::Create("MainEngineCamera");
    if (gameObjectID == GID_INVALID)
        return GID_INVALID;

    auto gameObject = GameObject::Get(gameObjectID);
    auto camera = gameObject->addComponent<Camera>();
    if (!camera)
        return GID_INVALID;

    return gameObjectID;
}

void Scene::setEnable(bool value)
{
    for (auto &gameObjectID : m_objects)
    {
        auto gameObject = GameObject::Get(gameObjectID);

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
    for (auto& gameObjectID : m_objects)
    {
        auto gameObject = GameObject::Get(gameObjectID);

        for_each(gameObject->getTransform(), func);
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
