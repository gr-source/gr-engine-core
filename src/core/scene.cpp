#include "gr-base/core/scene.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/logger.hpp"

#include "gr-base/ecs/archetypeStorage.hpp"
#include "gr-base/ecs/world.hpp"

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

Scene::Scene() : bRunning(false), m_ecs(nullptr), m_camera(INVALID_ENTITY)
{
    logger::info("[ Scene ] Create '0x%lX'.", this);

    m_ecs = new World();
}

Scene::~Scene()
{
    delete m_ecs;

    logger::info("[ Scene ] Destroy '0x%lX'.", this);
}

Entity Scene::create(const char* name)
{
    Entity e = m_ecs->Create();
    m_ecs->AddComponent<Transform>(e);
    if (name != nullptr)
        m_ecs->AddComponent<Name>(e, strdup(name));

    m_entities.push_back(e);

    return e;
}

Entity Scene::create(Entity other)
{
    if (other == INVALID_ENTITY)
        return create(nullptr);

    Entity entity = m_ecs->Create();

    auto go = Wrap(entity);
    const auto otherGO = Wrap(other);
    // m_isStatic = otherGO->m_isStatic;

    auto otherTrans = otherGO.GetComponent<Transform>();

    auto myTrans = go.GetOrAddComponent<Transform>();
    myTrans->position = otherTrans->position;
    myTrans->rotation = otherTrans->rotation;
    myTrans->scale = otherTrans->scale;
    myTrans->dirty = true;

    auto otherRel = otherGO.GetComponent<Relationship>();
    if (otherRel != nullptr)
    {
        Entity sibling = otherRel->firstChild;
        while (sibling != INVALID_ENTITY)
        {
            Entity newChild = create(sibling);
            SetParent(entity, newChild);

            auto siblingRel = m_ecs->GetComponent<Relationship>(sibling);
            if (siblingRel == nullptr)
                break;
            sibling = siblingRel->nextSibling;
        }
    }

    auto rel = go.GetComponent<Relationship>();
    if (rel == nullptr || rel->parent == INVALID_ENTITY)
        m_entities.push_back(go.entity);

    for (auto& typeID : m_ecs->GetAllComponentType(other))
    {
        auto type = Type::GetTypeInfoByID(typeID);

        if (go.GetComponent(typeID) != nullptr)
            continue;

        auto dst = go.AddComponent(typeID);
        auto src = otherGO.GetComponent(typeID);

        if (type->globalCpy)
        {
            memcpy(dst, src, type->stride);
        } else
        {
            ((void(*)(void*, const void*))(type->copy))(dst, src);
        }
    }

    return entity;
}

void Scene::destroy(Entity e)
{
    Unlink(e);

    m_ecs->Destroy(e);
}

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

void Scene::MarkChildrenDirty(Entity entity)
{
    auto transform = m_ecs->GetComponent<Transform>(entity);
    transform->dirty = true;

    auto rel = m_ecs->GetComponent<Relationship>(entity);
    if (rel != nullptr)
    {
        Entity sibling = rel->firstChild;
        while (sibling != INVALID_ENTITY)
        {
            MarkChildrenDirty(sibling);

            auto siblingRel = m_ecs->GetComponent<Relationship>(sibling);
            if (siblingRel == nullptr)
                break;
            sibling = siblingRel->nextSibling;
        }
    }
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
    }

    MarkChildrenDirty(child);
}

std::string Scene::GetEntityName(Entity entity)
{
    GameObject go = Wrap(entity);

    Name* name = go.GetComponent<Name>();
    if (name != nullptr)
    {
        return name->value;
    } else
    {
        EntityID entityID = handle_get_first(entity);

        return "Entity " + std::to_string(entityID);
    }
}



