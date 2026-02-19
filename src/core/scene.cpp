#include "gr-base/core/scene.hpp"

#include "LinearMath/btQuaternion.h"
#include "gr-base/core/default.hpp"
#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/gameObject.hpp"

// #include "gr-base/core/scenemanager.hpp"
#include "gr-base/core/thread/threadpool.hpp"
#include "gr-base/ecs/behaviorStorage.hpp"
#include "gr-base/ecs/default.hpp"
#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/gameobjectmanager.hpp"
#include "gr-base/managers/sceneManager.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

#include <algorithm>

// public
Scene* Scene::GetCurrent()
{
    return SceneManager::GetCurrent();
}

Scene::Scene() : bRunning(false), m_cameraID(INVALID_COMPONENT_ID) {}

Scene::~Scene()
{
    for (auto gameObject : getObjectList())
        GameObject::DestroyImediate(gameObject);
}

std::vector<GameObjectHandle> Scene::getObjectList() const
{
    return m_objects;
}

void Scene::insert(GameObjectHandle gameObjectID)
{
    auto it = std::find(m_objects.begin(), m_objects.end(), gameObjectID);
    if (it != m_objects.end())
    {
        return;
    }

    m_objects.push_back(gameObjectID);
}

void Scene::remove(GameObjectHandle gameObjectID)
{
    m_objects.erase(std::remove(m_objects.begin(), m_objects.end(), gameObjectID), m_objects.end());
}

void Scene::Clear()
{
    m_objects.clear();
}

void Scene::SetCamera(ComponentHandle id)
{
    m_cameraID = id;
}

Camera *Scene::getCamera() const
{
    return m_cameraID != INVALID_COMPONENT_ID ? BehaviorStorage::GetComponent<Camera>(m_cameraID) : nullptr;
}

// ********** public ********** //
void Scene::select(GameObjectHandle id)
{
    v_object_selected_list.push_back(id);

    u_object_has_been_selected.emplace(id);
}

void Scene::deselect(GameObjectHandle id)
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

bool Scene::object_has_been_selected(GameObjectHandle id) const
{
    return u_object_has_been_selected.find(id) != u_object_has_been_selected.end();
}

std::vector<GameObjectHandle> Scene::get_object_selected() const
{
    return v_object_selected_list;
}

bool Scene::destroy(GameObjectHandle gameObjectID)
{
    remove(gameObjectID);

    GameObject::DestroyImediate(gameObjectID);
    return true;
}

void Scene::AddLight(ComponentHandle id)
{
    if (Threadpool::IsMainThread())
        m_lights.push_back(id);
}

void Scene::RemoveLight(ComponentHandle id)
{
    if (Threadpool::IsMainThread())
        m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), id), m_lights.end());
}

std::vector<ComponentHandle> Scene::GetLights() const
{
    return m_lights;
}

void Scene::AddCamera(ComponentHandle id)
{
    if (Threadpool::IsMainThread())
    {
        m_cameras.push_back(id);

        if (m_cameraID == INVALID_COMPONENT_ID)
            SetCamera(id);
    }
}

void Scene::RemoveCamera(ComponentHandle id)
{
    if (Threadpool::IsMainThread())
    {
        m_cameras.erase(std::remove(m_cameras.begin(), m_cameras.end(), id), m_cameras.end());

        if (m_cameraID == id)
            m_cameraID = m_cameras.empty() ? INVALID_COMPONENT_ID : m_cameras[m_cameras.size() - 1];
    }
}

std::vector<ComponentHandle> Scene::GetCameras() const
{
    return m_cameras;
}

void Scene::Serialize(Stream* stream) const
{
    {
        uint64_t meshCount = static_cast<uint64_t>(m_meshes.size());
        if (!stream->write(meshCount))
            return;

        for (auto meshID : m_meshes)
        {
            auto meshKey = AssetsManager::GetMeshName(meshID);

            if (!stream->write<std::string>(meshKey))
                return;
        }
    }

    auto objects = getObjectList();

    uint64_t objectCount = static_cast<uint64_t>(objects.size());
    if (!stream->write(objectCount))
        return;

    for (GameObjectHandle gameObjectID : objects)
    {
        GameObject* gameObject = GameObject::Get(gameObjectID);
        gameObject->Serialize(stream);
    }
}

void Scene::Deserialize(Stream* stream)
{
    {
        uint64_t meshCount = 0;
        if (!stream->read(meshCount))
            return;

        for (uint64_t i=0;i<meshCount;i++)
        {
            std::string meshKey;
            if (!stream->read<std::string>(meshKey))
                return;

            std::cout << "Load: " << meshKey << std::endl;
        }
    }
    uint64_t objectCount = 0;
    if (!stream->read(objectCount))
        return;

    for (uint64_t i=0;i<objectCount;i++)
    {
        GameObjectHandle id = GameObject::Create("unknow");

        GameObject* gameObject = GameObject::Get(id);
        gameObject->Deserialize(stream);

        insert(id);
    }
}

void Scene::SetName(const std::string& name)
{
    m_name = name;
}

std::string Scene::GetName() const
{
    return m_name;
}

void Scene::AddMesh(MeshID meshID)
{
    m_meshes.push_back(meshID);
}

GameObjectHandle Scene::CreateEngineCamera()
{
    auto gameObjectID = GameObject::Create("MainEngineCamera");
    if (gameObjectID == HANDLE_INVALID)
        return HANDLE_INVALID;

    auto gameObject = GameObject::Get(gameObjectID);
    auto camera = gameObject->addComponent<Camera>();
    if (!camera)
        return HANDLE_INVALID;

    return gameObjectID;
}


