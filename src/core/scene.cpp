#include "gr-base/core/scene.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/logger.hpp"

#include "gr-base/ecs/archetypeStorage.hpp"
#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/world.hpp"

#include "gr-base/externals/handle.h"
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

Scene::Scene() : m_world(nullptr), m_root(INVALID_ENTITY), m_camera(INVALID_ENTITY), hierarchyDirty(false)
{
    logger::info("[ Scene ] Create '0x%lX'.", this);

    m_world = new World();

    m_root = m_world->Create();
    m_world->AddComponent<Relationship>(m_root);
}

Scene::~Scene()
{
    logger::info("[ Scene ] Destroy '0x%lX'.", this);

    delete m_world;
}

Entity Scene::create(const char* name)
{
    Entity e = m_world->Create();
    m_world->AddComponent<Transform>(e);
    if (name != nullptr)
        m_world->AddComponent<Name>(e, strdup(name));

    m_world->AddComponent<Relationship>(e);
    SetParent(m_root, e);

    return e;
}

Entity Scene::create(Entity other)
{
    if (other == INVALID_ENTITY)
        return create(nullptr);

    Entity entity = m_world->Create();

    auto go = Wrap(entity);
    const auto otherGO = Wrap(other);
    // m_isStatic = otherGO->m_isStatic;

    auto otherTrans = otherGO.GetComponent<Transform>();

    auto myTrans = go.GetOrAddComponent<Transform>();
    myTrans->position = otherTrans->position;
    myTrans->rotation = otherTrans->rotation;
    myTrans->scale = otherTrans->scale;
    MarkChanged<Transform>(go.entity);

    auto otherRel = otherGO.GetComponent<Relationship>();
    if (otherRel != nullptr)
    {
        Entity sibling = otherRel->firstChild;
        while (sibling != INVALID_ENTITY)
        {
            Entity newChild = create(sibling);
            SetParent(entity, newChild);

            auto siblingRel = m_world->GetComponent<Relationship>(sibling);
            if (siblingRel == nullptr)
                break;
            sibling = siblingRel->nextSibling;
        }
    }

    auto rel = go.GetComponent<Relationship>();
    if (rel == nullptr || rel->parent == INVALID_ENTITY)
        SetParent(m_root, go.entity);

    for (auto& typeID : m_world->GetAllComponentType(other))
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

    m_world->Destroy(e);
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

void Scene::SetName(const std::string& name)
{
    m_name = name;
}

std::string Scene::GetName() const
{
    return m_name;
}

size_t Scene::GetEntityCount() const
{
    return m_world->GetCount();
}

void Scene::MarkChildrenDirty(Entity entity)
{
    auto transform = m_world->GetComponent<Transform>(entity);
    if (transform != nullptr)
        MarkChanged<Transform>(entity);

    auto rel = m_world->GetComponent<Relationship>(entity);
    if (rel != nullptr)
    {
        Entity sibling = rel->firstChild;
        while (sibling != INVALID_ENTITY)
        {
            MarkChildrenDirty(sibling);

            auto siblingRel = m_world->GetComponent<Relationship>(sibling);
            if (siblingRel == nullptr)
                break;
            sibling = siblingRel->nextSibling;
        }
    }
}

void Scene::Unlink(Entity entity)
{
    auto rel = m_world->GetComponent<Relationship>(entity);
    if (rel == nullptr)
        return;

    Entity parent = rel->parent;

    if (parent != INVALID_ENTITY)
    {
        auto parentRel = m_world->GetComponent<Relationship>(parent);

        // se era o primeiro filho
        if (parentRel->firstChild == entity)
            parentRel->firstChild = rel->nextSibling;

        // conecta anterior -> próximo
        if (rel->prevSibling != INVALID_ENTITY)
        {
            auto prevRel = m_world->GetComponent<Relationship>(rel->prevSibling);

            prevRel->nextSibling = rel->nextSibling;
        }

        // conecta próximo -> anterior
        if (rel->nextSibling != INVALID_ENTITY)
        {
            auto nextRel = m_world->GetComponent<Relationship>(rel->nextSibling);

            nextRel->prevSibling = rel->prevSibling;
        }
    }

    rel->parent       = INVALID_ENTITY;
    rel->prevSibling  = INVALID_ENTITY;
    rel->nextSibling  = INVALID_ENTITY;

    hierarchyDirty = true;
}

void Scene::SetParent(Entity parent, Entity child)
{
    if (parent == child)
        return;

    m_world->GetOrAddComponent<Relationship>(child);
    m_world->GetOrAddComponent<Relationship>(parent);

    auto childRel  = m_world->GetComponent<Relationship>(child);
    auto parentRel = m_world->GetComponent<Relationship>(parent);

    if (childRel->parent != INVALID_ENTITY)
        Unlink(child);

    childRel->parent = parent;
    childRel->nextSibling = INVALID_ENTITY;
    childRel->prevSibling = INVALID_ENTITY;

    // Caso seja o primeiro filho
    if (parentRel->firstChild == INVALID_ENTITY)
    {
        parentRel->firstChild = child;
    } else
    {
        Entity oldFirst = parentRel->firstChild;

        auto oldFirstRel = m_world->GetComponent<Relationship>(oldFirst);

        // novo filho aponta pro antigo primeiro
        childRel->nextSibling = oldFirst;

        // antigo primeiro aponta de volta
        oldFirstRel->prevSibling = child;

        // parent agora aponta pro novo
        parentRel->firstChild = child;
    }

    hierarchyDirty = true;

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

void Scene::Update()
{
    for (int i=0;i<static_cast<int>(StageType::COUNT);i++)
    {
        for (auto& sys : m_systems[i])
            sys->Update(this);
    }
}


void Scene::MarkChanged(TypeID type, Entity e)
{
    if (!m_world->IsAlive(e))
        return;

    if (type >= changes.size())
        changes.resize(type + 1);

    uint32_t entityID = handle_get_first(e);

    ChangeSet& change = changes[type];
    if (entityID >= change.exists.size())
        change.exists.resize(entityID + 1, false);

    if (!change.exists[entityID])
    {
        change.entities.push_back(e);

        change.exists[entityID] = true;
    }
}

std::vector<Entity> Scene::Changed(TypeID type)
{
    if (type >= changes.size())
        return std::vector<Entity>();

    ChangeSet& change = changes[type];
    change.exists.clear();

    return std::move(change.entities);
}

