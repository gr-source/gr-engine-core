#include "gr-base/core/scene.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/default.hpp"
#include "gr-base/core/logger.hpp"

#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/spatial/bvhtree.hpp"

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

Scene::Scene() :
    world(new engine::ecs::world()),
    m_bvhtree(new BVHtree()),
    m_root(INVALID_ENTITY),
    m_camera(INVALID_ENTITY),
    hierarchyDirty(false)
{
    logger::info("[ Scene ] Create '%p'.", this);

    m_root = GetWorldDynamic()->Create();
    GetWorldDynamic()->AddComponent<Relationship>(m_root);
}

Scene::~Scene()
{
    logger::info("[ Scene ] Destroy '%p'.", this);

    for (auto& id : m_systemIDs)
    {
        void* system = m_typeIDToSystem[id];

        auto info = SystemRegistry::GetSystemByID(id);

        ((void (*)(void*))(info->destructor))(system);

        free(system);
    }

    if (m_bvhtree != nullptr)
        delete m_bvhtree;

    if (world != nullptr)
        delete world;
}

Entity Scene::create(const char* name)
{
    Entity e = GetWorldDynamic()->Create();
    GetWorldDynamic()->AddComponent<Transform>(e);
    if (name != nullptr)
        GetWorldDynamic()->AddComponent<Name>(e, strdup(name));
    //
    // GetWorldDynamic()->AddComponent<Relationship>(e);
    SetParent(m_root, e);

    return e;
}

Entity Scene::create(Entity other)
{
    if (other == INVALID_ENTITY)
        return INVALID_ENTITY;

    GameObject go(this, GetWorldDynamic()->Create());
    const GameObject otherGO(this, other);
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
            SetParent(go.entity, newChild);

            auto siblingRel = GetWorldDynamic()->GetComponent<Relationship>(sibling);
            if (siblingRel == nullptr)
                break;
            sibling = siblingRel->nextSibling;
        }
    }

    auto rel = go.GetComponent<Relationship>();
    if (rel == nullptr || rel->parent == INVALID_ENTITY)
        SetParent(m_root, go.entity);

    for (auto& typeID : GetWorldDynamic()->GetAllComponentType(other))
    {
        auto type = ComponentRegistry::GetComponentByID(typeID);

        if (go.HasComponentByID(typeID))
            continue;

        void* dst = go.GetOrAddComponentByID(typeID);
        void* src = otherGO.GetComponentByID(typeID);

        if (type->isTriviallyCopyable)
        {
            memcpy(dst, src, type->stride);
        } else
        {
            // o component foi inicializado em AddComponent
            ((void(*)(void*))(type->destructor))(dst);

            // Reinicializa com os dados novos copiados
            ((void(*)(void*, const void*))(type->copy))(dst, src);
        }
    }

    if (go.HasComponent<Transform, MeshFilter, MeshRender>())
    {
        auto meshFilter = go.GetComponent<MeshFilter>();
        if (meshFilter->mesh != HANDLE_INVALID)
        {
            auto mesh = AssetsManager::GetMesh(meshFilter->mesh);
            if (mesh != nullptr)
            {
                auto& bounds = mesh->localBounds;
                if (bounds.Volume() > 0.0f)
                    m_bvhtree->insert(go.entity, bounds);
            }
        }
    }

    return go.entity;
}

void Scene::destroy(Entity e)
{
    if (GetWorldDynamic()->HasComponent<Transform, MeshFilter, MeshRender>(e))
        m_bvhtree->remove(e);

    Unlink(e);

    GetWorldDynamic()->Destroy(e);
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

size_t Scene::GetEntityCount()
{
    return GetWorldDynamic()->GetEntityCount();
}

void Scene::MarkChildrenDirty(Entity entity)
{
    auto transform = GetWorldDynamic()->GetComponent<Transform>(entity);
    if (transform != nullptr)
        MarkChanged<Transform>(entity);

    auto rel = GetWorldDynamic()->GetComponent<Relationship>(entity);
    if (rel != nullptr)
    {
        Entity sibling = rel->firstChild;
        while (sibling != INVALID_ENTITY)
        {
            MarkChildrenDirty(sibling);

            auto siblingRel = GetWorldDynamic()->GetComponent<Relationship>(sibling);
            if (siblingRel == nullptr)
                break;
            sibling = siblingRel->nextSibling;
        }
    }
}

void Scene::Unlink(Entity entity)
{
    auto rel = GetWorldDynamic()->GetComponent<Relationship>(entity);
    if (rel == nullptr)
        return;

    Entity parent = rel->parent;

    if (parent != INVALID_ENTITY)
    {
        auto parentRel = GetWorldDynamic()->GetComponent<Relationship>(parent);

        // se era o primeiro filho
        if (parentRel->firstChild == entity)
            parentRel->firstChild = rel->nextSibling;

        // conecta anterior -> próximo
        if (rel->prevSibling != INVALID_ENTITY)
        {
            auto prevRel = GetWorldDynamic()->GetComponent<Relationship>(rel->prevSibling);

            prevRel->nextSibling = rel->nextSibling;
        }

        // conecta próximo -> anterior
        if (rel->nextSibling != INVALID_ENTITY)
        {
            auto nextRel = GetWorldDynamic()->GetComponent<Relationship>(rel->nextSibling);

            nextRel->prevSibling = rel->prevSibling;
        }
    }

    rel->parent       = INVALID_ENTITY;
    rel->prevSibling  = INVALID_ENTITY;
    rel->nextSibling  = INVALID_ENTITY;

    hierarchyDirty = true;
}

void Scene::SetParent(Entity parentEntity, Entity childEntity)
{
    if (parentEntity == childEntity)
        return;

    GameObject parent(this, parentEntity);
    GameObject child(this, childEntity);

    auto childRel  = child.GetOrAddComponent<Relationship>();
    auto parentRel = parent.GetOrAddComponent<Relationship>();

    if (childRel->parent != INVALID_ENTITY)
        Unlink(child.entity);

    childRel->parent = parent.entity;
    childRel->nextSibling = INVALID_ENTITY;
    childRel->prevSibling = INVALID_ENTITY;

    // Caso seja o primeiro filho
    if (parentRel->firstChild == INVALID_ENTITY)
    {
        parentRel->firstChild = child.entity;
    } else
    {
        Entity oldFirst = parentRel->firstChild;

        auto oldFirstRel = GetWorldDynamic()->GetComponent<Relationship>(oldFirst);

        // novo filho aponta pro antigo primeiro
        childRel->nextSibling = oldFirst;

        // antigo primeiro aponta de volta
        oldFirstRel->prevSibling = child.entity;

        // parent agora aponta pro novo
        parentRel->firstChild = child.entity;
    }

    hierarchyDirty = true;

    MarkChildrenDirty(child.entity);
}

#include "gr-base/scopeTimer.hpp"

void Scene::Update()
{
    for (auto& sys : m_systems)
        sys->Update(this);
    /*
    for (auto& systemID : m_systemIDs)
    {
        ISystem* sys = m_typeIDToSystem[systemID];

        auto info = SystemRegistry::GetSystemByID(systemID);

        ScopeTimer scope(info->name);

        sys->Update(this);
    }
    */

    // m_bvhtree->renderer();
}

void Scene::MarkChanged(TypeID type, Entity e)
{
    if (!GetWorldDynamic()->IsAlive(e))
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

void Scene::AddSystemByID(TypeID systemID)
{
    auto info = SystemRegistry::GetSystemByID(systemID);
    if (info == nullptr)
        return;

    if (info->index >= m_typeIDToSystem.size())
        m_typeIDToSystem.resize(info->index + 1, nullptr);

    if (m_typeIDToSystem[info->index] != nullptr)
        return;

    ISystem* system = createSystem(info);
    if (system == nullptr)
        return;

    m_systemIDs.push_back(info->index);


    m_typeIDToSystem[info->index] = system;

    m_systems.push_back(system);

    sortSystems();
}

void Scene::DelSystemByID(TypeID systemID)
{
    auto info = SystemRegistry::GetSystemByID(systemID);
    if (info == nullptr)
        return;

    if (info->index >= m_typeIDToSystem.size())
        return;

    if (m_typeIDToSystem[info->index] == nullptr)
        return;

    m_systemIDs.erase(std::remove(m_systemIDs.begin(), m_systemIDs.end(), info->index), m_systemIDs.end());

    ISystem* system = m_typeIDToSystem[info->index];
    m_typeIDToSystem[info->index] = nullptr;

    m_systems.erase(std::remove(m_systems.begin(), m_systems.end(), system), m_systems.end());

    info->destructor(system);
    free(system);

    sortSystems();
}

ISystem* Scene::createSystem(const SystemInfo* info)
{
    ISystem* system = (ISystem*)malloc(info->stride);
    info->constructor(system, nullptr);

    try
    {
        system->Init();
    } catch (const std::exception& e)
    {
        info->destructor(system);

        free(system);

        Console::LogError("[ Scene ] Fail in to create system='%s' error='%s'.", info->name, e.what());

        return nullptr;
    }

    return system;
}

void Scene::sortSystems()
{
    std::unordered_map<TypeID, Signature> graph;
    for (auto& id : m_systemIDs)
        graph[id] = SystemRegistry::GetSystemByID(id)->depends_on;

    std::unordered_set<TypeID> visited;
    std::unordered_set<TypeID> temp;
    std::vector<TypeID> order;
    bool hasCycle = false;

    std::function<void(TypeID)> dfs = [&](TypeID node)
    {
        if (temp.count(node))
        { 
            hasCycle = true; 
            return;
        }

        if (!visited.count(node))
        {
            temp.insert(node);

            Signature mask = graph[node];
            while (mask)
            {
                TypeID dep = __builtin_ctzll(mask);
                if (graph.find(dep) != graph.end())
                    dfs(dep);

                mask &= (mask - 1);
            }

            temp.erase(node);
            visited.insert(node);
            order.push_back(node);
        }
    };

    for (auto& id : m_systemIDs)
    {
        if (!visited.count(id))
            dfs(id);
    }

    if (hasCycle)
    {
        std::cout << "Erro: ciclo detectado na DAG!\n";
        return;
    }

    for (size_t i=0;i<order.size();i++)
    {
        m_systems[i] = m_typeIDToSystem[order[i]];
        m_systemIDs[i] = order[i];
    }
}



