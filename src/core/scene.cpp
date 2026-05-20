#include "gr-base/core/scene.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/ecs.hpp"

#include "gr-base/managers/sceneManager.hpp"

#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/name.hpp"

#include <algorithm>
#include <cstring>

// public
Scene* Scene::GetCurrent()
{
    return SceneManager::GetCurrent();
}

Scene::Scene() : bRunning(false), m_ecs(std::make_shared<ECS>()), m_camera(INVALID_ENTITY) {}

Entity Scene::create(const char* name)
{
    Entity e = m_ecs->Create();
    m_ecs->AddComponent<Transform>(e);
    m_ecs->AddComponent<Name>(e, strdup(name));

    m_entities.push_back(e);

    return e;
}

void Scene::destroy(Entity e)
{}

void Scene::delFromRoot(Entity e)
{
    m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), e), m_entities.end());
}

// ********** public ********** //
void Scene::select(Handle id)
{
    v_object_selected_list.push_back(id);

    u_object_has_been_selected.emplace(id);
}

void Scene::deselect(Handle id)
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

bool Scene::object_has_been_selected(Handle id) const
{
    return u_object_has_been_selected.find(id) != u_object_has_been_selected.end();
}

std::vector<Handle> Scene::get_object_selected() const
{
    return v_object_selected_list;
}



// void Scene::AddLight(ComponentHandle id)
// {
//     if (Threadpool::IsMainThread())
//         m_lights.push_back(id);
// }

// void Scene::RemoveLight(ComponentHandle id)
// {
//     if (Threadpool::IsMainThread())
//         m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), id), m_lights.end());
// }

// std::vector<ComponentHandle> Scene::GetLights() const
// {
//     return m_lights;
// }
//
// void Scene::AddCamera(ComponentHandle id)
// {
//     if (Threadpool::IsMainThread())
//     {
//         m_cameras.push_back(id);
//
//         if (m_cameraID == INVALID_COMPONENT_ID)
//             SetCamera(id);
//     }
// }
//
// void Scene::RemoveCamera(ComponentHandle id)
// {
//     if (Threadpool::IsMainThread())
//     {
//         m_cameras.erase(std::remove(m_cameras.begin(), m_cameras.end(), id), m_cameras.end());
//
//         if (m_cameraID == id)
//             m_cameraID = m_cameras.empty() ? INVALID_COMPONENT_ID : m_cameras[m_cameras.size() - 1];
//     }
// }
//
// std::vector<ComponentHandle> Scene::GetCameras() const
// {
//     return m_cameras;
// }

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


size_t Scene::GetEntityCount() const
{
    return m_ecs->GetCount();
}

void Scene::Unlink(Entity entity)
{
    auto rel = m_ecs->GetComponent<Relationship>(entity);
    if (rel == nullptr)
        return;

    Entity parent = rel->parent;

    if (parent != INVALID_ENTITY)
    {
        auto parentRel = m_ecs->GetComponent<Relationship>(parent);

        if (parentRel->firstChild == entity)
        {
            parentRel->firstChild = rel->nextSibling;
        }
        else
        {
            Entity sibling = parentRel->firstChild;

            while (sibling != INVALID_ENTITY)
            {
                auto siblingRel = m_ecs->GetComponent<Relationship>(sibling);

                if (siblingRel->nextSibling == entity)
                {
                    siblingRel->nextSibling = rel->nextSibling;
                    break;
                }

                sibling = siblingRel->nextSibling;
            }
        }
    }

    rel->parent = INVALID_ENTITY;
    rel->firstChild = INVALID_ENTITY;
    rel->nextSibling = INVALID_ENTITY;
}

void Scene::SetParent(Entity parent, Entity child)
{
    m_ecs->GetOrAddComponent<Relationship>(child);
    m_ecs->GetOrAddComponent<Relationship>(parent);

    auto childRel  = m_ecs->GetComponent<Relationship>(child);
    auto parentRel = m_ecs->GetComponent<Relationship>(parent);

    if (childRel->parent == INVALID_ENTITY)
    {
        delFromRoot(child);
    }

    childRel->parent = parent;

    if (parentRel->firstChild == INVALID_ENTITY)
    {
        parentRel->firstChild = child;
    } else
    {
        Entity sibling = parentRel->firstChild;

        while (sibling != INVALID_ENTITY)
        {
            auto siblingRel = m_ecs->GetOrAddComponent<Relationship>(sibling);

            if (siblingRel->nextSibling == INVALID_ENTITY)
            {
                siblingRel->nextSibling = child;
                break;
            }

            sibling = siblingRel->nextSibling;
        }

        /*
        Entity sibling = parentRel->firstChild;
        while (m_ecs->GetOrAddComponent<Relationship>(sibling)->nextSibling != INVALID_ENTITY)
            sibling = m_ecs->GetOrAddComponent<Relationship>(sibling)->nextSibling;

        auto siblingRel = m_ecs->GetOrAddComponent<Relationship>(sibling);
        siblingRel->nextSibling = child;
        */
    }
}


